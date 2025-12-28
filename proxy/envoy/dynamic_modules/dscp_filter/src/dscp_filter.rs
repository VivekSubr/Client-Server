use envoy_proxy_dynamic_modules_rust_sdk::*;

// Socket option constants for setting DSCP/TOS
// IPPROTO_IP = 0, IP_TOS = 1 (for IPv4)
// IPPROTO_IPV6 = 41, IPV6_TCLASS = 67 (for IPv6)
const IPPROTO_IP: i32 = 0;
const IP_TOS: i32 = 1;

// FFI declaration for the socket option callback
extern "C" {
    fn envoy_dynamic_module_callback_http_set_socket_option(
        filter_envoy_ptr: abi::envoy_dynamic_module_type_http_filter_envoy_ptr,
        level: i32,
        optname: i32,
        optval: *const std::ffi::c_void,
        optlen: usize,
    ) -> bool;
}

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
    envoy_filter_config: &mut EC,
    filter_name: &str,
    filter_config: &[u8],
) -> Option<Box<dyn HttpFilterConfig<EC, EHF>>> {
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

impl<EC: EnvoyHttpFilterConfig, EHF: EnvoyHttpFilter> HttpFilterConfig<EC, EHF> for FilterConfig {
    /// This is called for each new HTTP filter.
    fn new_http_filter(&mut self, _envoy_config: &mut EC) -> Box<dyn HttpFilter<EHF>> {
        Box::new(Filter::new())
    }
}

/// This implements the [`envoy_proxy_dynamic_modules_rust_sdk::HttpFilter`] trait.
///
/// This filter reads the 'x-dscp' header from client requests and sets it in the response.
pub struct Filter {
    /// Stores the DSCP value from the request header
    dscp_value: Option<Vec<u8>>,
}

impl Filter {
    pub fn new() -> Self {
        Self {
            dscp_value: None,
        }
    }

    /// Set the DSCP/TOS value on the downstream socket
    /// DSCP values are typically 0-63, but the TOS field is 8 bits where DSCP is the upper 6 bits
    /// So DSCP value needs to be shifted left by 2: TOS = DSCP << 2
    /// 
    /// Safety: envoy_filter must be a valid EnvoyHttpFilterImpl which has raw_ptr as its first field
    unsafe fn set_socket_dscp<EHF: EnvoyHttpFilter>(envoy_filter: &mut EHF, dscp_value: u8) -> bool {
        // EnvoyHttpFilterImpl is a repr(Rust) struct with raw_ptr as its only field
        // We can safely read the first field by treating the reference as a pointer to the raw_ptr
        let ptr: abi::envoy_dynamic_module_type_http_filter_envoy_ptr = 
            *(envoy_filter as *mut EHF as *mut abi::envoy_dynamic_module_type_http_filter_envoy_ptr);
        
        // TOS byte = DSCP (6 bits) << 2 | ECN (2 bits, typically 0)
        let tos_value: i32 = (dscp_value as i32) << 2;
        let result = envoy_dynamic_module_callback_http_set_socket_option(
            ptr,
            IPPROTO_IP,
            IP_TOS,
            &tos_value as *const i32 as *const std::ffi::c_void,
            std::mem::size_of::<i32>(),
        );
        if result {
            println!("Successfully set socket IP_TOS to {} (DSCP {})", tos_value, dscp_value);
        } else {
            eprintln!("Failed to set socket IP_TOS");
        }
        result
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
        // Read the 'x-dscp' header from the client request
        if let Some(dscp_buffer) = envoy_filter.get_request_header_value("x-dscp") {
            let dscp_bytes = dscp_buffer.as_slice().to_vec();
            println!(
                "Received x-dscp header from request: {}",
                String::from_utf8_lossy(&dscp_bytes)
            );
            self.dscp_value = Some(dscp_bytes);
        } else {
            println!("No x-dscp header found in request");
            self.dscp_value = None;
        }

        abi::envoy_dynamic_module_type_on_http_filter_request_headers_status::Continue
    }

    fn on_response_headers(
        &mut self,
        envoy: &mut EHF,
        _end_stream: bool,
    ) -> abi::envoy_dynamic_module_type_on_http_filter_response_headers_status {
        // Set the x-dscp value from the request into the response header and socket
        if let Some(ref dscp_value) = self.dscp_value {
            // Set the response header
            let success = envoy.set_response_header("x-dscp", dscp_value);
            if success {
                println!(
                    "Set x-dscp response header to: {}",
                    String::from_utf8_lossy(dscp_value)
                );
            } else {
                eprintln!("Failed to set x-dscp response header");
            }

            // Parse DSCP value and set on socket
            if let Ok(dscp_str) = std::str::from_utf8(dscp_value) {
                if let Ok(dscp_num) = dscp_str.trim().parse::<u8>() {
                    if dscp_num <= 63 {
                        // DSCP is 6 bits, max value 63
                        unsafe {
                            Self::set_socket_dscp(envoy, dscp_num);
                        }
                    } else {
                        eprintln!("DSCP value {} out of range (0-63)", dscp_num);
                    }
                } else {
                    eprintln!("Failed to parse DSCP value: {}", dscp_str);
                }
            }
        } else {
            // No DSCP value from request, set a default or skip
            let success = envoy.set_response_header("x-dscp", b"not-specified");
            if !success {
                eprintln!("Failed to set default x-dscp response header");
            }
        }

        // Add a marker header to indicate the filter processed the request
        let success = envoy.set_response_header("x-dscp-filter", b"processed");
        if !success {
            eprintln!("Failed to set x-dscp-filter response header");
        }

        abi::envoy_dynamic_module_type_on_http_filter_response_headers_status::Continue
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    /// Test filter captures x-dscp header from request
    fn test_filter_captures_dscp_header() {
        let mut envoy_filter = envoy_proxy_dynamic_modules_rust_sdk::MockEnvoyHttpFilter::new();

        // Mock getting the x-dscp request header
        envoy_filter
            .expect_get_request_header_value()
            .withf(|key| key == "x-dscp")
            .times(1)
            .returning(|_| {
                // Return a mock buffer with DSCP value "46"
                Some(envoy_proxy_dynamic_modules_rust_sdk::EnvoyBuffer::new(b"46"))
            });

        let mut filter = Filter::new();

        // Process request headers - should capture x-dscp
        assert_eq!(
            filter.on_request_headers(&mut envoy_filter, false),
            abi::envoy_dynamic_module_type_on_http_filter_request_headers_status::Continue
        );

        // Verify the DSCP value was stored
        assert_eq!(filter.dscp_value, Some(b"46".to_vec()));
    }

    #[test]
    /// Test filter handles missing x-dscp header
    fn test_filter_without_dscp_header() {
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
        assert_eq!(filter.dscp_value, None);
    }
}