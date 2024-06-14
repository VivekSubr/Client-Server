#include <string>
#include <string_view>
#include <unordered_map>

#include "proxy_wasm_intrinsics.h" //https://github.com/proxy-wasm/proxy-wasm-cpp-sdk/blob/main/proxy_wasm_intrinsics.h

class ExampleRootContext : public RootContext {
public:
  explicit ExampleRootContext(uint32_t id, std::string_view root_id) : RootContext(id, root_id) {}

  bool onStart(size_t) override;
  bool onConfigure(size_t) override;
  void onTick() override;
};

class ExampleContext : public Context {
public:
  explicit ExampleContext(uint32_t id, RootContext* root) : Context(id, root) {}

  void onCreate() override;
  FilterStatus onNewConnection() override;
  void onDone() override;
  void onLog() override;
  void onDelete() override;
};

static RegisterContextFactory register_ExampleContext(CONTEXT_FACTORY(ExampleContext),
                                                      ROOT_FACTORY(ExampleRootContext),
                                                      "my_root_id");

bool ExampleRootContext::onStart(size_t) {
  LOG_TRACE("onStart");
  return true;
}

bool ExampleRootContext::onConfigure(size_t) {
  LOG_TRACE("onConfigure");
  proxy_set_tick_period_milliseconds(1000); // 1 sec
  return true;
}

void ExampleRootContext::onTick() { LOG_TRACE("onTick"); }

void ExampleContext::onDone() { LOG_WARN(std::string("onDone " + std::to_string(id()))); }

void ExampleContext::onLog() { LOG_WARN(std::string("onLog " + std::to_string(id()))); }

void ExampleContext::onDelete() { LOG_WARN(std::string("onDelete " + std::to_string(id()))); }


void ExampleContext::onCreate() { LOG_WARN(std::string("onCreate " + std::to_string(id()))); }

FilterStatus ExampleContext::onNewConnection() 
{
  LOG_WARN("dscp filter - onNewConnection");
  return FilterStatus::Continue;
}