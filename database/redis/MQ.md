# Redis as a Message Queue
Redis Streams are used to achieve pub-sub like functionality, streams are an append only datastructure in redis.

## Streams vs Traditional Pub/Sub
**Traditional Pub/Sub:**
    * Fire-and-forget (no persistence)
    * Messages lost if no subscriber is listening
    * No message history
    * Simple fan-out pattern

**Streams:**
    * Messages are persisted
    * Consumers can read from any point in the stream
    * Built-in consumer groups for load balancing
    * Acknowledgment and pending message tracking
    * Can replay message history

## Basic Operations 

**Write to Stream**
```
    # Add a message (Redis generates ID)
    XADD mystream * sensor_id 1234 temperature 19.8 humidity 65

    # Returns: "1619181234567-0"

    # Add with explicit ID
    XADD mystream 1619181234567-1 sensor_id 1235 temperature 20.1

    # Add with max length (keep only last 1000 entries)
    XADD mystream MAXLEN ~ 1000 * event "user_login" user_id 42
```

**Reading from Stream**
```
    # Read last 10 entries
    XREAD COUNT 10 STREAMS mystream 0

    # Block for up to 5000ms waiting for new messages
    XREAD BLOCK 5000 STREAMS mystream $

    # Read from multiple streams
    XREAD COUNT 2 STREAMS stream1 stream2 0-0 0-0

    # Read only new messages ($ means "from now")
    XREAD BLOCK 0 STREAMS mystream $
```

**Range queries**
```
    # Read all messages
    XRANGE mystream - +

    # Read specific time range
    XRANGE mystream 1619181234000 1619181235000

    # Read in reverse
    XREVRANGE mystream + - COUNT 5
```

Using basic XADD, every consumer can poll XREAD and achieve pub/sub in a basic way.

## Consumer Groups
Consumer groups allows more fine grained pub/sub by allowing subs to acknowledge they have consumed a message.
```
    # Create group "processors" starting from beginning
    XGROUP CREATE mystream processors 0

    # Consumer "worker-1" reads 2 messages from group "processors"
    XREADGROUP GROUP processors worker-1 COUNT 2 STREAMS mystream >

    # Returns pending messages for this consumer
    XREADGROUP GROUP processors worker-1 STREAMS mystream 0

    # Block waiting for new messages
    XREADGROUP GROUP processors worker-1 BLOCK 2000 STREAMS mystream >
```

The > symbol means "give me new messages that haven't been delivered to any consumer in this group." 

Subs should ACK messages as well.
```
    # Acknowledge processed messages
    XACK mystream processors 1619181234567-0 1619181234567-1

    # Check pending messages
    XPENDING mystream processors

    # Claim abandoned messages (from dead consumers)
    XCLAIM mystream processors worker-2 3600000 1619181234567-0
```