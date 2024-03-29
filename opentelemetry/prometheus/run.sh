set -x

rm -f otel.log prom.log
pkill prometheus
pkill otel

#https://opentelemetry.io/docs/collector/getting-started/
docker run --name otel -p 4317:4317 \
    -v $(pwd)/otel_config.yaml:/etc/otelcol/config.yaml \
    otel/opentelemetry-collector:0.71.0 &> otel.log & 

docker run --name prometheus \
    -p 9090:9090 \
    -v $(pwd)/prometheus.yaml:/etc/prometheus/prometheus.yml \
    prom/prometheus &> prom.log &
