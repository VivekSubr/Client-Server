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
  S0([Setup]) --> S1[DROP/CREATE test<br/>INSERT 2]
  S1 --> S2[pg_advisory_unlock_all]
  
  %% Parallel start - both sessions begin
  S2 --> T1A
  S2 --> T2A
 
  %% T1 Startup: Acquire gate locks + readiness polling
  T1A[T1: lock 1, 3, 5, 6] --> T1B[T1 readiness poll:<br/>loop: try_lock 7<br/>if success: unlock 7, sleep 0.05<br/>exit when try_lock 7 fails<br/>meaning T2 holds lock 7]
 
  %% T2 Startup: Acquire gate locks + readiness polling
  T2A[T2: lock 2, 4, 7] --> T2B[T2 readiness poll:<br/>loop: try_lock 6<br/>if success: unlock 6, sleep 0.05<br/>exit when try_lock 6 fails<br/>meaning T1 holds lock 6]
 
  %% Begin transactions
  T1B --> T1C[T1: BEGIN<br/>READ COMMITTED]
  T2B --> T2C[T2: BEGIN<br/>REPEATABLE READ<br/>snapshot NOT yet fixed]
 
  %% === Barrier 1: T1 step 8 → T2 waits ===
  T1C --> T1S1[T1: SELECT val → sees 2]
  T1S1 --> T1U1[T1: unlock 1]
  
  T2C --> T2W1[T2: wait on lock 1]
  T1U1 --> T2W1
  T2W1 --> T2U1[T2: unlock 1]
 
  %% === Barrier 2: T2 step 9 → T1 waits ===
  T2U1 --> T2S1[T2: INSERT 4<br/>uncommitted]
  T2S1 --> T2U2[T2: unlock 2]
  
  T1U1 --> T1W2[T1: wait on lock 2]
  T2U2 --> T1W2
  T1W2 --> T1U2[T1: unlock 2]
 
  %% === T1 step 10 ===
  T1U2 --> T1S2[T1: SELECT val → sees 2<br/>T2's INSERT 4 not visible<br/>not committed yet]
 
  %% === Barrier 3: T1 step 11 → T2 waits ===
  T1S2 --> T1S3[T1: INSERT 5<br/>uncommitted]
  T1S3 --> T1U3[T1: unlock 3]
  
  T2U2 --> T2W3[T2: wait on lock 3]
  T1U3 --> T2W3
  T2W3 --> T2U3[T2: unlock 3]
 
  %% === Barrier 4: T2 step 12 → T1 waits ===
  T2U3 --> T2S2[T2: SELECT val → sees 2, 4<br/>SNAPSHOT FIXED HERE<br/>sees initial 2 + own INSERT 4]
  T2S2 --> T2U4[T2: unlock 4]
  
  T1U3 --> T1W4[T1: wait on lock 4]
  T2U4 --> T1W4
  T1W4 --> T1U4[T1: unlock 4]
 
  %% === Barrier 5: T1 step 13 → T2 waits ===
  T1U4 --> T1S4[T1: COMMIT<br/>row 5 now visible<br/>to new READ COMMITTED snapshots]
  T1S4 --> T1U5[T1: unlock 5]
  
  T2U4 --> T2W5[T2: wait on lock 5]
  T1U5 --> T2W5
  T2W5 --> T2U5[T2: unlock 5]
 
  %% === T2 step 14 ===
  T2U5 --> T2S3[T2: SELECT val → still sees 2, 4<br/>REPEATABLE READ isolation<br/>snapshot ignores T1's committed 5]
 
  %% === T2 step 15 ===
  T2S3 --> T2S4[T2: COMMIT]
  
  %% End
  T1U5 --> END([Test Complete])
  T2S4 --> END
 
  %% Styling
  classDef t1Style fill:#e1f5ff,stroke:#0066cc,stroke-width:2px
  classDef t2Style fill:#fff4e1,stroke:#cc6600,stroke-width:2px
  classDef setupStyle fill:#f0f0f0,stroke:#666,stroke-width:2px
  classDef barrierStyle fill:#ffe1e1,stroke:#cc0000,stroke-width:2px
  
  class T1A,T1B,T1C,T1S1,T1U1,T1W2,T1U2,T1S2,T1S3,T1U3,T1W4,T1U4,T1S4,T1U5 t1Style
  class T2A,T2B,T2C,T2W1,T2U1,T2S1,T2U2,T2W3,T2U3,T2S2,T2U4,T2W5,T2U5,T2S3,T2S4 t2Style
  class S0,S1,S2,END setupStyle
```
 
## Key Observations
 
1. **T2's snapshot is fixed at its FIRST SELECT** (step 12), not at BEGIN
2. **Barrier synchronization** ensures precise interleaving via advisory locks
3. **T1 (READ COMMITTED)** sees committed changes immediately
4. **T2 (REPEATABLE READ)** maintains snapshot consistency - ignores T1's COMMIT of row 5
5. **Readiness polling** ensures both sessions hold their gate locks before starting the interleaved phase
