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

```mermaid
flowchart TD
  %% Setup
  S0([Setup]) --> S1[DROP/CREATE test; INSERT 2]
  S1 --> S2[pg_advisory_unlock_all]
  
  %% Parallel start
  S2 --> T1A
  S2 --> T2A
 
  %% T1 Startup
  T1A[T1: lock 1,3,5,6] --> T1B[T1 readiness: loop try_lock 7<br/>if success unlock 7, sleep<br/>exit when try_lock 7 fails]
 
  %% T2 Startup
  T2A[T2: lock 2,4,7] --> T2B[T2 readiness: loop try_lock 6<br/>if success unlock 6, sleep<br/>exit when try_lock 6 fails]
 
  %% Begin transactions
  T1B --> T1C[T1: BEGIN READ COMMITTED]
  T2B --> T2C[T2: BEGIN REPEATABLE READ<br/>snapshot fixed]
 
  %% Interleaving via barriers
  T1C --> T1S8[T1 Step 8: SELECT → 2]
  T1S8 --> U1[T1: unlock 1]
  U1 --> T2W1[T2: lock 1 unblocks]
  T2W1 --> T2PU1[T2: unlock 1]
 
  T2PU1 --> T2S9[T2 Step 9: INSERT 4<br/>uncommitted]
  T2S9 --> U2[T2: unlock 2]
  U2 --> T1W2[T1: lock 2 unblocks]
  T1W2 --> T1PU2[T1: unlock 2]
 
  T1PU2 --> T1S10[T1 Step 10: SELECT → 2]
  T1S10 --> T1S11[T1 Step 11: INSERT 5<br/>uncommitted]
  T1S11 --> U3[T1: unlock 3]
  U3 --> T2W3[T2: lock 3 unblocks]
  T2W3 --> T2PU3[T2: unlock 3]
 
  T2PU3 --> T2S12[T2 Step 12: SELECT → 2,4]
  T2S12 --> U4[T2: unlock 4]
  U4 --> T1W4[T1: lock 4 unblocks]
  T1W4 --> T1PU4[T1: unlock 4]
 
  T1PU4 --> T1S13[T1 Step 13: COMMIT]
  T1S13 --> U5[T1: unlock 5]
  U5 --> T2W5[T2: lock 5 unblocks]
  T2W5 --> T2PU5[T2: unlock 5]
 
  T2PU5 --> T2S14[T2 Step 14: SELECT → still 2,4]
  T2S14 --> T2S15[T2 Step 15: COMMIT]
```