use envoy_proxy_dynamic_modules_rust_sdk::*;

// Socket option constants for setting DSCP/TOS
// IPPROTO_IP = 0, IP_TOS = 1 (for IPv4)
// IPPROTO_IPV6 = 41, IPV6_TCLASS = 67 (for IPv6)
const IPPROTO_IP: i64 = 0;
const IP_TOS: i64 = 1;

declare_init_functions!(init, new_http_filter_config_fn);

/// This implements the [`envoy_proxy_dynamic_modules_rust_sdk::ProgramInitFunction`].
///
/// This is called exactly once when the module is loaded. It can be used to
/// initialize global state as well as check the runtime environment to ensure that
/// the module is running in a supported environment.
///
/// Returning `false` will cause Envoy to reject the config hence the
/// filter will not be loaded.
fn init() -> bool {
    true
}

/// This implements the [`envoy_proxy_dynamic_modules_rust_sdk::NewHttpFilterConfigFunction`].
///
/// This is the entrypoint every time a new HTTP filter is created via the DynamicModuleFilter config.
///
/// Each argument matches the corresponding argument in the Envoy config here:
/// https://www.envoyproxy.io/docs/envoy/latest/api-v3/extensions/dynamic_modules/v3/dynamic_modules.proto#envoy-v3-api-msg-extensions-dynamic-modules-v3-dynamicmoduleconfig
///
/// Returns None if the filter name or config is determined to be invalid by each filter's `new` function.
fn new_http_filter_config_fn<EC: EnvoyHttpFilterConfig, EHF: EnvoyHttpFilter>(
    _envoy_filter_config: &mut EC,
    _filter_name: &str,
    filter_config: &[u8],
) -> Option<Box<dyn HttpFilterConfig<EHF>>> {
    let filter_config = std::str::from_utf8(filter_config).unwrap();
    Some(Box::new(FilterConfig::new(filter_config)))
}

/// This implements the [`envoy_proxy_dynamic_modules_rust_sdk::HttpFilterConfig`] trait.
///
/// The trait corresponds to a Envoy filter chain configuration.
pub struct FilterConfig {
    _filter_config: String,
}

impl FilterConfig {
    /// This is the constructor for the [`FilterConfig`].
    ///
    /// filter_config is the filter config from the Envoy config here:
    /// https://www.envoyproxy.io/docs/envoy/latest/api-v3/extensions/dynamic_modules/v3/dynamic_modules.proto#envoy-v3-api-msg-extensions-dynamic-modules-v3-dynamicmoduleconfig
    pub fn new(filter_config: &str) -> Self {
        Self {
            _filter_config: filter_config.to_string(),
        }
    }
}

impl<EHF: EnvoyHttpFilter> HttpFilterConfig<EHF> for FilterConfig {
    /// This is called for each new HTTP filter.
    fn new_http_filter(&self, _envoy: &mut EHF) -> Box<dyn HttpFilter<EHF>> {
        Box::new(Filter::new())
    }
}

/// This implements the [`envoy_proxy_dynamic_modules_rust_sdk::HttpFilter`] trait.
///
/// This filter reads the 'x-dscp' header from client requests and sets the DSCP/TOS
/// socket option on the upstream connection. It also reads 'x-dscp' from server
/// responses and sets DSCP on the downstream socket for responses back to the client.
pub struct Filter {
    /// Stores the DSCP value from the request header (for upstream)
    request_dscp_value: Option<Vec<u8>>,
    /// Stores the DSCP value from the response header (for downstream)
    response_dscp_value: Option<Vec<u8>>,
}

impl Filter {
    pub fn new() -> Self {
        Self {
            request_dscp_value: None,
            response_dscp_value: None,
        }
    }

    /// Helper function to parse DSCP value and set socket option
    fn set_dscp_socket_option<EHF: EnvoyHttpFilter>(
        envoy_filter: &mut EHF,
        dscp_bytes: &[u8],
        state: abi::envoy_dynamic_module_type_socket_option_state,
        direction: abi::envoy_dynamic_module_type_socket_direction,
        direction_label: &str,
    ) -> bool {
        if let Ok(dscp_str) = std::str::from_utf8(dscp_bytes) {
            if let Ok(dscp_num) = dscp_str.trim().parse::<u8>() {
                if dscp_num <= 63 {
                    // DSCP is 6 bits, max value 63
                    // TOS byte = DSCP (6 bits) << 2 | ECN (2 bits, typically 0)
                    let tos_value: i64 = (dscp_num as i64) << 2;

                    let result = envoy_filter.set_socket_option_int(
                        IPPROTO_IP,
                        IP_TOS,
                        state,
                        direction,
                        tos_value,
                    );

                    if result {
                        println!(
                            "[{}] Successfully set socket IP_TOS to {} (DSCP {})",
                            direction_label, tos_value, dscp_num
                        );
                        return true;
                    } else {
                        eprintln!("[{}] Failed to set socket IP_TOS", direction_label);
                    }
                } else {
                    eprintln!("[{}] DSCP value {} out of range (0-63)", direction_label, dscp_num);
                }
            } else {
                eprintln!("[{}] Failed to parse DSCP value: {}", direction_label, dscp_str);
            }
        }
        false
    }
}

/// This implements the [`envoy_proxy_dynamic_modules_rust_sdk::HttpFilter`] trait.
///
/// Default implementation of all methods is to return `Continue`.
impl<EHF: EnvoyHttpFilter> HttpFilter<EHF> for Filter {
    fn on_request_headers(
        &mut self,
        envoy_filter: &mut EHF,
        _end_of_stream: bool,
    ) -> abi::envoy_dynamic_module_type_on_http_filter_request_headers_status {
        // Read the 'x-dscp' header from the client request and set socket option immediately
        // Socket options must be set BEFORE the upstream connection is created
        if let Some(dscp_buffer) = envoy_filter.get_request_header_value("x-dscp") {
            let dscp_bytes = dscp_buffer.as_slice().to_vec();
            println!(
                "[Request] Received x-dscp header: {}",
                String::from_utf8_lossy(&dscp_bytes)
            );

            // Set DSCP on upstream socket using Prebind state
            Self::set_dscp_socket_option(
                envoy_filter,
                &dscp_bytes,
                abi::envoy_dynamic_module_type_socket_option_state::Prebind,
                abi::envoy_dynamic_module_type_socket_direction::Upstream,
                "Upstream",
            );

            self.request_dscp_value = Some(dscp_bytes);
        } else {
            println!("[Request] No x-dscp header found");
            self.request_dscp_value = None;
        }

        abi::envoy_dynamic_module_type_on_http_filter_request_headers_status::Continue
    }

    fn on_response_headers(
        &mut self,
        envoy_filter: &mut EHF,
        _end_stream: bool,
    ) -> abi::envoy_dynamic_module_type_on_http_filter_response_headers_status {
        // Check if server sent x-dscp header in response
        if let Some(dscp_buffer) = envoy_filter.get_response_header_value("x-dscp") {
            let dscp_bytes = dscp_buffer.as_slice().to_vec();
            println!(
                "[Response] Received x-dscp header from server: {}",
                String::from_utf8_lossy(&dscp_bytes)
            );

            // Set DSCP on downstream socket (for response to client)
            // Use Bound state since the downstream connection is already established
            Self::set_dscp_socket_option(
                envoy_filter,
                &dscp_bytes,
                abi::envoy_dynamic_module_type_socket_option_state::Bound,
                abi::envoy_dynamic_module_type_socket_direction::Downstream,
                "Downstream",
            );

            self.response_dscp_value = Some(dscp_bytes);
        } else {
            // Log if request had DSCP but response doesn't
            if let Some(ref req_dscp) = self.request_dscp_value {
                println!(
                    "[Response] No x-dscp from server (request had DSCP: {})",
                    String::from_utf8_lossy(req_dscp)
                );
            }
        }

        abi::envoy_dynamic_module_type_on_http_filter_response_headers_status::Continue
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    /// Test filter captures x-dscp header from request and sets upstream DSCP
    fn test_filter_captures_request_dscp_header() {
        let mut envoy_filter = envoy_proxy_dynamic_modules_rust_sdk::MockEnvoyHttpFilter::new();

        // Mock getting the x-dscp request header
        envoy_filter
            .expect_get_request_header_value()
            .withf(|key| key == "x-dscp")
            .times(1)
            .returning(|_| {
                // Return a mock buffer with DSCP value "46"
                Some(envoy_proxy_dynamic_modules_rust_sdk::EnvoyBuffer::new("46"))
            });

        // Mock set_socket_option_int call for upstream
        envoy_filter
            .expect_set_socket_option_int()
            .times(1)
            .returning(|_, _, _, _, _| true);

        let mut filter = Filter::new();

        // Process request headers - should capture x-dscp
        assert_eq!(
            filter.on_request_headers(&mut envoy_filter, false),
            abi::envoy_dynamic_module_type_on_http_filter_request_headers_status::Continue
        );

        // Verify the DSCP value was stored
        assert_eq!(filter.request_dscp_value, Some(b"46".to_vec()));
    }

    #[test]
    /// Test filter handles missing x-dscp header in request
    fn test_filter_without_request_dscp_header() {
        let mut envoy_filter = envoy_proxy_dynamic_modules_rust_sdk::MockEnvoyHttpFilter::new();

        // Mock getting the x-dscp request header - returns None
        envoy_filter
            .expect_get_request_header_value()
            .withf(|key| key == "x-dscp")
            .times(1)
            .returning(|_| None);

        let mut filter = Filter::new();

        // Process request headers - no x-dscp present
        assert_eq!(
            filter.on_request_headers(&mut envoy_filter, false),
            abi::envoy_dynamic_module_type_on_http_filter_request_headers_status::Continue
        );

        // Verify no DSCP value was stored
        assert_eq!(filter.request_dscp_value, None);
    }

    #[test]
    /// Test filter captures x-dscp header from response and sets downstream DSCP
    fn test_filter_captures_response_dscp_header() {
        let mut envoy_filter = envoy_proxy_dynamic_modules_rust_sdk::MockEnvoyHttpFilter::new();

        // Mock getting the x-dscp response header
        envoy_filter
            .expect_get_response_header_value()
            .withf(|key| key == "x-dscp")
            .times(1)
            .returning(|_| {
                // Return a mock buffer with DSCP value "34"
                Some(envoy_proxy_dynamic_modules_rust_sdk::EnvoyBuffer::new("34"))
            });

        // Mock set_socket_option_int call for downstream
        envoy_filter
            .expect_set_socket_option_int()
            .times(1)
            .returning(|_, _, _, _, _| true);

        let mut filter = Filter::new();

        // Process response headers - should capture x-dscp from server
        assert_eq!(
            filter.on_response_headers(&mut envoy_filter, false),
            abi::envoy_dynamic_module_type_on_http_filter_response_headers_status::Continue
        );

        // Verify the response DSCP value was stored
        assert_eq!(filter.response_dscp_value, Some(b"34".to_vec()));
    }

    #[test]
    /// Test filter handles missing x-dscp header in response
    fn test_filter_without_response_dscp_header() {
        let mut envoy_filter = envoy_proxy_dynamic_modules_rust_sdk::MockEnvoyHttpFilter::new();

        // Mock getting the x-dscp response header - returns None
        envoy_filter
            .expect_get_response_header_value()
            .withf(|key| key == "x-dscp")
            .times(1)
            .returning(|_| None);

        let mut filter = Filter::new();

        // Process response headers - no x-dscp present
        assert_eq!(
            filter.on_response_headers(&mut envoy_filter, false),
            abi::envoy_dynamic_module_type_on_http_filter_response_headers_status::Continue
        );

        // Verify no response DSCP value was stored
        assert_eq!(filter.response_dscp_value, None);
    }
}
