# Data Migration and Backup

**Kafka MirrorMaker 2** continuously copies Kafka data between clusters. It is built on the Kafka Connect framework.

It runs three connector types:

• MirrorSourceConnector: Replicates topic records and configurations.
• MirrorCheckpointConnector: Maps consumer-group offsets to the target cluster.
• MirrorHeartbeatConnector: Publishes heartbeats to verify connectivity and replication flow.

Example:

```
kind: KafkaMirrorMaker2
spec:
  connectCluster: target
  clusters:
    - alias: source
      bootstrapServers: old-kafka:9092
    - alias: target
      bootstrapServers: new-kafka:9092
  mirrors:
    - sourceCluster: source
      targetCluster: target
      sourceConnector:
        tasksMax: 4
      checkpointConnector: {}
      heartbeatConnector: {}
```

This configures the mm2 pod to continously copy from source to target.