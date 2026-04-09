# Concurency in PostGres

**Advisory locks**
Advisory locks in PostgreSQL are application-level locks that you explicitly manage in your code... but these at the DB level rather than in app itself.

In test_transaction_isolation, we do 
    ```
    SELECT pg_advisory_lock(1);
    SELECT pg_advisory_lock(3);
    SELECT pg_advisory_lock(5);
    SELECT pg_advisory_lock(6);
    ```

pg_advisory_lock(1) is non-blocking for the caller — the calling session acquires the lock and immediately continues to the next statement. It does *not* block itself.

It only blocks other sessions that try to pg_advisory_lock(1) while you hold it. Those sessions will wait (hang) on that call until you release it with pg_advisory_unlock(1).
ie, the next  SELECT pg_advisory_lock(1) will block till pg_advisory_unlock(1) is done by the owning session.

flowchart TD
  %% ----------------------------
  %% Setup (single session)
  %% ----------------------------
  S0([Setup]) --> S1[DROP/CREATE test; INSERT (2)]
  S1 --> S2[pg_advisory_unlock_all()]

  %% ----------------------------
  %% Parallel start
  %% ----------------------------
  S2 --> P0{Start both sessions concurrently}

  %% ----------------------------
  %% T1 Startup
  %% ----------------------------
  P0 --> T1A[T1: lock(1), lock(3), lock(5), lock(6)]
  T1A --> T1B[T1: Wait for T2 readiness<br/>Loop: try_lock(7)<br/>if success: unlock(7), sleep<br/>exit when try_lock(7) fails (means T2 holds 7)]

  %% ----------------------------
  %% T2 Startup
  %% ----------------------------
  P0 --> T2A[T2: lock(2), lock(4), lock(7)]
  T2A --> T2B[T2: Wait for T1 readiness<br/>Loop: try_lock(6)<br/>if success: unlock(6), sleep<br/>exit when try_lock(6) fails (means T1 holds 6)]

  %% ----------------------------
  %% Both begin transactions after rendezvous
  %% ----------------------------
  T1B --> T1C[T1: BEGIN; ISO=READ COMMITTED]
  T2B --> T2C[T2: BEGIN; ISO=REPEATABLE READ<br/>(snapshot fixed at BEGIN)]

  %% ----------------------------
  %% Barrier/Step interleaving
  %% ----------------------------
  T1C --> T1S8[T1 Step 8: SELECT -> expects [2]]
  T1S8 --> U1[T1: unlock(1)] --> T2W1[T2: lock(1) wait ends] --> T2PU1[T2: unlock(1)]

  T2PU1 --> T2S9[T2 Step 9: INSERT (4) uncommitted]
  T2S9 --> U2[T2: unlock(2)] --> T1W2[T1: lock(2) wait ends] --> T1PU2[T1: unlock(2)]

  T1PU2 --> T1S10[T1 Step 10: SELECT -> still [2] (can't see uncommitted 4)]
  T1S10 --> T1S11[T1 Step 11: INSERT (5) uncommitted]
  T1S11 --> U3[T1: unlock(3)] --> T2W3[T2: lock(3) wait ends] --> T2PU3[T2: unlock(3)]

  T2PU3 --> T2S12[T2 Step 12: SELECT -> [2,4]<br/>(snapshot 2 + own insert 4)]
  T2S12 --> U4[T2: unlock(4)] --> T1W4[T1: lock(4) wait ends] --> T1PU4[T1: unlock(4)]

  T1PU4 --> T1S13[T1 Step 13: COMMIT (5 becomes committed/visible)]
  T1S13 --> U5[T1: unlock(5)] --> T2W5[T2: lock(5) wait ends] --> T2PU5[T2: unlock(5)]

  T2PU5 --> T2S14[T2 Step 14: SELECT -> still [2,4]<br/>(repeatable read ignores T1 commit 5)]
  T2S14 --> T2S15[T2 Step 15: COMMIT]