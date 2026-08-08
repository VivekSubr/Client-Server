# Install Kafka on WSL

Run these commands inside an Ubuntu WSL terminal. This setup installs Kafka 4.0.0 in KRaft mode, so ZooKeeper is not required.

## 1. Install Java and download tools

```bash
sudo apt update
sudo apt install -y openjdk-17-jdk curl
java -version
```

## 2. Download Kafka

```bash
KAFKA_VERSION=4.0.0
SCALA_VERSION=2.13

mkdir -p "$HOME/apps"
cd "$HOME/apps"
curl -fLO "https://archive.apache.org/dist/kafka/${KAFKA_VERSION}/kafka_${SCALA_VERSION}-${KAFKA_VERSION}.tgz"
tar -xzf "kafka_${SCALA_VERSION}-${KAFKA_VERSION}.tgz"
ln -sfn "kafka_${SCALA_VERSION}-${KAFKA_VERSION}" kafka

export KAFKA_HOME="$HOME/apps/kafka"
export PATH="$KAFKA_HOME/bin:$PATH"
```

To make the environment variables available in future WSL terminals:

```bash
printf '\nexport KAFKA_HOME="$HOME/apps/kafka"\nexport PATH="$KAFKA_HOME/bin:$PATH"\n' >> "$HOME/.bashrc"
source "$HOME/.bashrc"
```

## 3. Initialize Kafka storage

Use a directory under your WSL home folder so Kafka data is not stored in `/tmp`.

```bash
cd "$KAFKA_HOME"
mkdir -p "$HOME/kafka-data"
sed -i "s|^log.dirs=.*|log.dirs=$HOME/kafka-data|" config/server.properties

KAFKA_CLUSTER_ID="$(bin/kafka-storage.sh random-uuid)"
bin/kafka-storage.sh format --standalone \
	--cluster-id "$KAFKA_CLUSTER_ID" \
	--config config/server.properties
```

Only format the storage directory once. Running the format command again against existing data is unnecessary.

## 4. Start Kafka

```bash
cd "$KAFKA_HOME"
bin/kafka-server-start.sh config/server.properties
```

Keep this terminal open. Kafka listens on `localhost:9092` by default.

## 5. Verify the installation

Open a second WSL terminal and run:

```bash
source "$HOME/.bashrc"

kafka-topics.sh --create \
	--topic quickstart-events \
	--bootstrap-server localhost:9092

kafka-topics.sh --describe \
	--topic quickstart-events \
	--bootstrap-server localhost:9092
```

Start a producer and enter a few messages, pressing Enter after each one:

```bash
kafka-console-producer.sh \
	--topic quickstart-events \
	--bootstrap-server localhost:9092
```

In a third WSL terminal, read the messages:

```bash
source "$HOME/.bashrc"
kafka-console-consumer.sh \
	--topic quickstart-events \
	--from-beginning \
	--bootstrap-server localhost:9092
```

Press `Ctrl+C` to stop the producer, consumer, or Kafka server.
