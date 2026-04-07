#!/bin/bash

DB_NAME="testdb"
DB_USER="postgres"
PASSED=0
FAILED=0

check_and_start_postgresql() { #Expect pgsql to be installed
    if sudo service postgresql status > /dev/null 2>&1; then
        echo "PostgreSQL is already running."
    else
        echo "PostgreSQL is not running. Starting..."
        sudo service postgresql start
    fi
}

run_sql() {
    sudo -u "$DB_USER" psql -d "$DB_NAME" -t -A -c "$1" 2>/dev/null
}

assert_eq() {
    local description="$1"
    local expected="$2"
    local actual="$3"
    if [[ "$actual" == "$expected" ]]; then
        echo "  PASS: $description"
        ((PASSED++))
    else
        echo "  FAIL: $description (expected='$expected', got='$actual')"
        ((FAILED++))
    fi
}

# --- Setup ---

setup() {
    echo "=== Setup ==="
    check_and_start_postgresql
    # Create DB using default 'postgres' database (can't connect to testdb to create it)
    sudo -u "$DB_USER" psql -d postgres -t -A -c "CREATE DATABASE $DB_NAME;" 2>/dev/null
    run_sql "DROP TABLE IF EXISTS test_table;"
    run_sql "CREATE TABLE test_table (id SERIAL PRIMARY KEY, name VARCHAR(100), value INT);"
    echo ""
}

# --- Test Cases ---

extract_between() {
    local file="$1" start_marker="$2" end_marker="$3"
    sed -n "/$start_marker/,/$end_marker/p" "$file" | grep -E '^[0-9]+$'
}

test_transaction_isolation() {
    echo "=== Test: Transaction Isolation (READ COMMITTED vs REPEATABLE READ) ==="

    # Setup: create table with initial row
    run_sql "DROP TABLE IF EXISTS test;"
    run_sql "CREATE TABLE test (val INT);"
    run_sql "INSERT INTO test VALUES (2);"
    run_sql "SELECT pg_advisory_unlock_all();"
    echo "  Inserted initial row (2)"

    local tmpdir
    tmpdir=$(mktemp -d)
    chmod 777 "$tmpdir"

    # Synchronization via advisory locks:
    #   Barrier 1: T1 holds → releases after step 8  → T2 waits before step 9
    #   Barrier 2: T2 holds → releases after step 9  → T1 waits before step 10
    #   Barrier 3: T1 holds → releases after step 11 → T2 waits before step 12
    #   Barrier 4: T2 holds → releases after step 12 → T1 waits before step 13
    #   Barrier 5: T1 holds → releases after step 13 → T2 waits before step 14
    #
    # Startup: Both sessions start, acquire their gate locks, signal readiness
    # via marker files, then wait for a "go" file before proceeding. This
    # ensures both hold their locks before either enters the interleaved phase.

    # T1 (unquoted heredoc for $tmpdir expansion)
    cat > "$tmpdir/t1.sql" << SQL
-- Acquire gate locks T1 controls
SELECT pg_advisory_lock(1);
SELECT pg_advisory_lock(3);
SELECT pg_advisory_lock(5);
\\! touch $tmpdir/t1_ready
\\! while [ ! -f $tmpdir/go ]; do sleep 0.1; done

BEGIN;
SET TRANSACTION ISOLATION LEVEL READ COMMITTED;

-- Step 8: T1 SELECT
\\echo ---STEP8---
SELECT val FROM test ORDER BY val;
\\echo ---END8---
SELECT pg_advisory_unlock(1);

-- Wait for T2 step 9 (INSERT 4)
SELECT pg_advisory_lock(2);
SELECT pg_advisory_unlock(2);

-- Step 10: T1 SELECT (T2's INSERT not yet committed)
\\echo ---STEP10---
SELECT val FROM test ORDER BY val;
\\echo ---END10---

-- Step 11: T1 INSERT (5)
INSERT INTO test VALUES (5);
SELECT pg_advisory_unlock(3);

-- Wait for T2 step 12
SELECT pg_advisory_lock(4);
SELECT pg_advisory_unlock(4);

-- Step 13: T1 COMMIT
COMMIT;
SELECT pg_advisory_unlock(5);
SQL

    # T2 (unquoted heredoc for $tmpdir expansion)
    cat > "$tmpdir/t2.sql" << SQL
-- Acquire gate locks T2 controls
SELECT pg_advisory_lock(2);
SELECT pg_advisory_lock(4);
\\! touch $tmpdir/t2_ready
\\! while [ ! -f $tmpdir/go ]; do sleep 0.1; done

BEGIN;
SET TRANSACTION ISOLATION LEVEL REPEATABLE READ;

-- Wait for T1 step 8
SELECT pg_advisory_lock(1);
SELECT pg_advisory_unlock(1);

-- Step 9: T2 INSERT (4)
INSERT INTO test VALUES (4);
SELECT pg_advisory_unlock(2);

-- Wait for T1 step 11
SELECT pg_advisory_lock(3);
SELECT pg_advisory_unlock(3);

-- Step 12: T2 SELECT (snapshot + own insert)
\\echo ---STEP12---
SELECT val FROM test ORDER BY val;
\\echo ---END12---
SELECT pg_advisory_unlock(4);

-- Wait for T1 step 13 (commit)
SELECT pg_advisory_lock(5);
SELECT pg_advisory_unlock(5);

-- Step 14: T2 SELECT (REPEATABLE READ ignores T1's commit)
\\echo ---STEP14---
SELECT val FROM test ORDER BY val;
\\echo ---END14---

-- Step 15: T2 COMMIT
COMMIT;
SQL

    chmod 644 "$tmpdir/t1.sql" "$tmpdir/t2.sql"

    echo "  Running T1 (READ COMMITTED) and T2 (REPEATABLE READ) concurrently..."

    # Launch both sessions
    sudo -u "$DB_USER" psql -d "$DB_NAME" -t -A -f "$tmpdir/t1.sql" > "$tmpdir/out1" 2>&1 &
    local pid1=$!
    sudo -u "$DB_USER" psql -d "$DB_NAME" -t -A -f "$tmpdir/t2.sql" > "$tmpdir/out2" 2>&1 &
    local pid2=$!

    # Wait for both sessions to confirm they hold their gate locks
    while [[ ! -f "$tmpdir/t1_ready" ]] || [[ ! -f "$tmpdir/t2_ready" ]]; do
        if ! kill -0 "$pid1" 2>/dev/null; then
            echo "  ERROR: T1 died during startup:"; cat "$tmpdir/out1"; return 1
        fi
        if ! kill -0 "$pid2" 2>/dev/null; then
            echo "  ERROR: T2 died during startup:"; cat "$tmpdir/out2"; return 1
        fi
        sleep 0.1
    done

    # Both hold their locks — release the barrier
    touch "$tmpdir/go"

    wait $pid1; local rc1=$?
    wait $pid2; local rc2=$?

    if [[ $rc1 -ne 0 ]]; then
        echo "  ERROR: T1 failed (exit $rc1):"
        cat "$tmpdir/out1"
    fi
    if [[ $rc2 -ne 0 ]]; then
        echo "  ERROR: T2 failed (exit $rc2):"
        cat "$tmpdir/out2"
    fi

    # Parse results (extract_between filters to numeric-only lines)
    local step8 step10 step12 step14
    step8=$(extract_between "$tmpdir/out1" "STEP8" "END8")
    step10=$(extract_between "$tmpdir/out1" "STEP10" "END10")
    step12=$(extract_between "$tmpdir/out2" "STEP12" "END12")
    step14=$(extract_between "$tmpdir/out2" "STEP14" "END14")

    echo ""
    echo "  Step 8:  T1 SELECT           → $step8"
    echo "  Step 10: T1 SELECT           → $step10"
    echo "  Step 12: T2 SELECT           → $(echo "$step12" | tr '\n' ',')"
    echo "  Step 14: T2 SELECT (post-T1) → $(echo "$step14" | tr '\n' ',')"
    echo ""

    assert_eq "Step 8:  T1 sees initial data (2)" "2" "$step8"
    assert_eq "Step 10: T1 sees (2), T2's (4) not committed" "2" "$step10"
    assert_eq "Step 12: T2 sees snapshot + own insert (2,4)" $'2\n4' "$step12"
    assert_eq "Step 14: T2 still sees (2,4), not T1's committed (5)" $'2\n4' "$step14"

    # Cleanup
    run_sql "SELECT pg_advisory_unlock_all();"
    run_sql "DROP TABLE IF EXISTS test;"
    rm -rf "$tmpdir"
}

test_million_random_integers() {
    echo "=== Test: Create table of 1 million random integers ==="

    run_sql "DROP TABLE IF EXISTS million_ints;"
    run_sql "CREATE TABLE million_ints (id SERIAL PRIMARY KEY, val INT);"
    run_sql "INSERT INTO million_ints (val) SELECT floor(random() * 2147483647)::INT FROM generate_series(1, 1000000);"
    local rc=$?

    if [[ $rc -ne 0 ]]; then
        echo "  ERROR: Insert failed (exit $rc)"
        ((FAILED++))
        return 1
    fi

    local count
    count=$(run_sql "SELECT count(*) FROM million_ints;")
    assert_eq "Table has 1000000 rows" "1000000" "$count"

    local distinct
    distinct=$(run_sql "SELECT count(DISTINCT val) FROM million_ints;")
    echo "  Distinct values: $distinct / 1000000"

    # Cleanup
    run_sql "DROP TABLE IF EXISTS million_ints;"
}

test_index_performance() {
    echo "=== Test: Index performance — Seq Scan vs B-tree vs Bitmap ==="

    run_sql "DROP TABLE IF EXISTS employees;"
    run_sql "CREATE TABLE employees (id SERIAL PRIMARY KEY, name TEXT, active BOOLEAN);"

    # Create a function to generate random strings
    run_sql "
CREATE OR REPLACE FUNCTION random_string(length integer) RETURNS text AS \$\$
declare
  chars text[] := '{0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z}';
  result text := '';
  i integer := 0;
  length2 integer := (select trunc(random() * length + 1));
begin
  if length2 < 0 then
    raise exception 'Given length cannot be less than 0';
  end if;
  for i in 1..length2 loop
    result := result || chars[1+random()*(array_length(chars, 1)-1)];
  end loop;
  return result;
end;
\$\$ language plpgsql;
"

    echo "  Inserting 1M rows (random name + boolean active)..."
    run_sql "INSERT INTO employees (name, active)
        SELECT random_string(10), (random() > 0.5)
        FROM generate_series(1, 1000000);"

    local count
    count=$(run_sql "SELECT count(*) FROM employees;")
    assert_eq "Table has 1000000 rows" "1000000" "$count"

    # Pick a name to search for (exact match — good for B-tree)
    local search_name
    search_name=$(run_sql "SELECT name FROM employees LIMIT 1;")
    echo ""
    echo "  --- Exact match: name = '$search_name' ---"

    # 1) Sequential scan (no index)
    local plan_seq time_seq
    plan_seq=$(run_sql "EXPLAIN SELECT * FROM employees WHERE name = '$search_name';")
    time_seq=$(run_sql "EXPLAIN ANALYZE SELECT * FROM employees WHERE name = '$search_name';" | grep "Execution Time" | awk '{print $3}')
    echo "  Seq Scan (no index):  ${time_seq} ms"
    echo "    Plan: $(echo "$plan_seq" | head -1)"

    # 2) B-tree index (default) — best for exact match / range queries
    run_sql "CREATE INDEX idx_btree_name ON employees USING btree (name);"
    echo ""
    echo "  Created B-tree index on employees(name)"
    local plan_btree time_btree
    plan_btree=$(run_sql "EXPLAIN SELECT * FROM employees WHERE name = '$search_name';")
    time_btree=$(run_sql "EXPLAIN ANALYZE SELECT * FROM employees WHERE name = '$search_name';" | grep "Execution Time" | awk '{print $3}')
    echo "  B-tree index:         ${time_btree} ms"
    echo "    Plan: $(echo "$plan_btree" | head -1)"

    if echo "$plan_btree" | grep -qi "index"; then
        echo "  PASS: B-tree query uses index scan"
        ((PASSED++))
    else
        echo "  FAIL: B-tree query does not use index scan"
        ((FAILED++))
    fi

    run_sql "DROP INDEX idx_btree_name;"

    # 3) Bitmap index scan — triggered by queries matching many rows (low cardinality)
    #    The 'active' boolean column has only 2 distinct values (~50% each),
    #    so Postgres uses a Bitmap Heap Scan when indexed.
    echo ""
    echo "  --- Low cardinality: active = true (~50% of rows) ---"

    local time_seq_bool
    time_seq_bool=$(run_sql "EXPLAIN ANALYZE SELECT count(*) FROM employees WHERE active = true;" | grep "Execution Time" | awk '{print $3}')
    echo "  Seq Scan (no index):  ${time_seq_bool} ms"

    run_sql "CREATE INDEX idx_btree_active ON employees USING btree (active);"
    echo "  Created B-tree index on employees(active)"

    # Force index use (Postgres may prefer seq scan for 50% selectivity)
    run_sql "SET enable_seqscan = off;"

    local plan_bitmap time_bitmap
    plan_bitmap=$(run_sql "EXPLAIN SELECT count(*) FROM employees WHERE active = true;")
    time_bitmap=$(run_sql "EXPLAIN ANALYZE SELECT count(*) FROM employees WHERE active = true;" | grep "Execution Time" | awk '{print $3}')
    echo "  Bitmap Index Scan:    ${time_bitmap} ms"
    echo "    Plan: $(echo "$plan_bitmap" | head -1)"

    run_sql "SET enable_seqscan = on;"

    if echo "$plan_bitmap" | grep -qi "bitmap"; then
        echo "  PASS: Low-cardinality query uses Bitmap scan"
        ((PASSED++))
    else
        echo "  FAIL: Low-cardinality query does not use Bitmap scan"
        ((FAILED++))
    fi

    # Summary
    echo ""
    echo "  --- Summary ---"
    echo "  Exact match (name):       Seq=${time_seq}ms  B-tree=${time_btree}ms"
    echo "  Low cardinality (active): Seq=${time_seq_bool}ms  Bitmap=${time_bitmap}ms"
    echo ""
    echo "  B-tree: best for high-cardinality, exact match, range queries"
    echo "  Bitmap: Postgres builds a bitmap of matching heap pages at query time;"
    echo "          efficient when an index matches many rows (low selectivity)"

    # Cleanup
    run_sql "DROP TABLE IF EXISTS employees;"
    run_sql "DROP FUNCTION IF EXISTS random_string(integer);"
}

# --- Teardown ---

teardown() {
    echo ""
    echo "=== Teardown ==="
    run_sql "DROP TABLE IF EXISTS test_table;"
    echo "Cleaned up test_table."
}

ALL_TESTS=(
    test_transaction_isolation
    test_million_random_integers
    test_index_performance
)

# --- Run All Tests ---

main() {
    setup

    if [[ $# -gt 0 ]]; then
        # Run specific tests passed as arguments
        for name in "$@"; do
            # Add test_ prefix if not present
            [[ "$name" != test_* ]] && name="test_$name"
            if declare -f "$name" > /dev/null 2>&1; then
                "$name"
            else
                echo "ERROR: Unknown test '$name'"
                echo "Available tests: ${ALL_TESTS[*]}"
                ((FAILED++))
            fi
        done
    else
        # Run all tests
        for t in "${ALL_TESTS[@]}"; do
            "$t"
        done
    fi

    teardown

    echo ""
    echo "=== Results: $PASSED passed, $FAILED failed ==="
    [[ "$FAILED" -eq 0 ]] && exit 0 || exit 1
}

main "$@"
