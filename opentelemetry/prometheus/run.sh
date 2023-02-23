set -x

#https://opentelemetry.io/docs/collector/getting-started/
docker run -v $(pwd)/otel_config.yaml:/etc/otelcol/config.yaml otel/opentelemetry-collector:0.71.0

docker run \
    -p 9090:9090 \
    -v $(pwd)/prometheus.yaml:/etc/prometheus/prometheus.yml \
    prom/prometheus