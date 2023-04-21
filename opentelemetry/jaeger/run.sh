set -x

rm -f otel.log jaeger.log
docker kill otel
docker kill jaeger 

#https://opentelemetry.io/docs/collector/getting-started/
docker run --name otel -p 4317:4317 \
    -v $(pwd)/otel_config.yaml:/etc/otelcol/config.yaml \
    otel/opentelemetry-collector:latest &> otel.log & 

#https://medium.com/jaegertracing/introducing-native-support-for-opentelemetry-in-jaeger-eb661be8183c
docker run --name jaeger \
    -e COLLECTOR_OTLP_ENABLED=true \
    -p 16686:16686 \
    -p 4318:4318 \
    jaegertracing/all-in-one:latest &> jaeger.log &