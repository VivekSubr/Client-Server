# PostgreSQL Test Cases

The test harness in `test_pg.sh` demonstrates transaction isolation, data generation, index selection, full-text search, and a manual inverted index.

## Running the Tests

PostgreSQL must be installed, and the current user must be able to run commands as the `postgres` operating-system user.

Run every test:

```bash
bash ./test_pg.sh
```

Run one or more selected tests by passing names with or without the `test_` prefix:

```bash
bash ./test_pg.sh inverted_index
bash ./test_pg.sh transaction_isolation full_text_search
```

The harness creates the `testdb` database if needed, reports each assertion as `PASS` or `FAIL`, cleans up its test tables, and exits with a nonzero status if any assertion fails.

## `test_transaction_isolation`

Demonstrates the difference between `READ COMMITTED` and `REPEATABLE READ` using two concurrent PostgreSQL sessions. Advisory locks make their operations execute in a deterministic order.

- T1 starts a `READ COMMITTED` transaction and initially reads `2`.
- T2 starts a `REPEATABLE READ` transaction and inserts `4` without committing.
- T1 reads again and still sees only `2` because T2 has not committed.
- T1 inserts `5`.
- T2 sees its original snapshot plus its own insert: `2,4`.
- T1 commits, but T2 continues to see `2,4` because its snapshot does not change under `REPEATABLE READ`.

The test asserts the result of each read and removes the temporary `test` table afterward.

## `test_million_random_integers`

Creates `million_ints` and inserts one million pseudo-random 32-bit positive integers with `generate_series` and `random`.

The test asserts that exactly 1,000,000 rows were inserted and prints the number of distinct values. It then drops the table.

## `test_index_performance`

Creates one million `employees` rows with a random text name and a roughly evenly distributed boolean `active` value. This is the slowest and most resource-intensive test.

It demonstrates two planner choices:

1. **Sequential scan versus B-tree:** An exact name lookup is measured without an index, then repeated after creating a B-tree index. The test verifies that the indexed query plan contains an index scan.
2. **Sequential scan versus bitmap scan:** A query for `active = true` matches about half the table. After indexing the low-cardinality boolean column, sequential scans are disabled for that session to demonstrate a bitmap scan. The test verifies that the plan contains a bitmap operation.

Execution times are informational because they vary by machine. The test asserts the row count and plan types, then removes the table and random-string helper function.

## `test_full_text_search`

Loads the records in `example_full_text_search_data.json` into `user_summaries`. A generated `tsvector` column normalizes each summary with the English text-search configuration, and a GIN index supports term lookups.

The test showcases:

- A single-term search for `engineer`.
- A ranked AND query for `machine & learning` using `ts_rank`.
- An OR query for `cloud | kubernetes` with matching terms highlighted by `ts_headline`.

Each query must match at least one seeded row. The test also inspects the query plan for GIN index use; PostgreSQL choosing a sequential scan for this small data set is reported as a note rather than a failure. The table is dropped afterward.

## `test_inverted_index`

Builds an inverted index explicitly so its term-to-document mapping is visible. Three rows in `documents` are tokenized and normalized to lowercase. The resulting `inverted_index` table stores one posting per `(term, document_id)` and an integer array containing every position of that term in the document.

The test verifies that:

- The posting list for `inverted` is `{2,3}`.
- The repeated term `index` occurs at positions `{3,5}` in document 3.
- Intersecting the postings for `inverted` and `search` implements an AND query and returns document 3.
- A term lookup uses the primary-key index on `(term, document_id)`.

Both `inverted_index` and `documents` are removed after the assertions.