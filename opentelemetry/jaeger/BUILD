cc_library(
   name="tracer",
   srcs=["tracer.cc"],
   copts =[
            "-Ilibs/curl-7.81.0/include",
            "-Ihttp2/libcurl_client",
   ],
   hdrs=["tracer.h", "http.h"],
   deps=[
        "@io_opentelemetry_cpp//api",
        "@io_opentelemetry_cpp//exporters/jaeger:opentelemetry_exporter_jaeger_trace",
        "@io_opentelemetry_cpp//sdk/src/trace",
   ]
)

cc_binary(
   name="example.exe",
   srcs=["example.cc"],
   deps = [
        "//opentracing/jaeger:tracer",
    ],
)