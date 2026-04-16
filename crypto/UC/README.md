UC uses cert manager. As part of install/upgrade, cert-manager CRO is created which in turn creates Secrets, using installed ClusterIssuers.


# Cert-Manager

## Components
** Certificate **
A Kubernetes custom resource that defines your desired certificate. You specify the domain names, issuer to use, and where to store the resulting secret.

** Issuer/ClusterIssuer **
Defines how certificates should be obtained. An Issuer works within a single namespace, while ClusterIssuer works cluster-wide. Common types:
    Let's Encrypt (ACME protocol)
    HashiCorp Vault
    Self-signed certificates
    CA (your own certificate authority)

** CertificateRequest **
Automatically created by cert-manager when a Certificate resource is created. It represents the actual request to the issuer.


In UC default setup, we have 
* CluserIssuer pointing to a secret in fed-paas-helpers
* Certificate CRO for cert-manager, which uses this issuer to create a secret.

Istio gateway resource is supposed to be configured to use this resource.
```
  servers:
  - hosts:
    - '*'
    port:
      name: https
      number: 3000
      protocol: HTTPS
    tls:
      credentialName: example-com-ingress-cert-manager
      maxProtocolVersion: TLSV1_3
      minProtocolVersion: TLSV1_2
      mode: MUTUAL
```

Istio configure this via Envoy's xDS api... basically each envoy sub-system has it's own 'discovery system', in this case Istio will use LDS (Listener DS) for envoy to open listener on port 3000, and SDS (Secret DS) to configure mTLS using example-com-ingress-cert-manager on it.


## Envoy

* Downstream config, Envoy is Server
```
{
  "transport_socket": {
    "name": "envoy.transport_sockets.tls",
    "typed_config": {
      "@type": "DownstreamTlsContext",
      "common_tls_context": {
        "tls_certificates": [{
          "certificate_chain": { "inline_string": "-----BEGIN CERTIFICATE-----..." },
          "private_key": { "inline_string": "-----BEGIN PRIVATE KEY-----..." }
        }],
        "validation_context": {
          "trusted_ca": { "inline_string": "-----BEGIN CERTIFICATE-----..." }
        },
        "tls_params": {
          "tls_minimum_protocol_version": "TLSv1_2",
          "tls_maximum_protocol_version": "TLSv1_3",
          "cipher_suites": ["ECDHE-RSA-AES128-GCM-SHA256", "..."]
        }
      },
      "require_client_certificate": true  // For MUTUAL TLS
    }
  }
}```


* Upstream config, Envoy is Client
```
{
  "transport_socket": {
    "name": "envoy.transport_sockets.tls",
    "typed_config": {
      "@type": "UpstreamTlsContext",
      "common_tls_context": {
        "validation_context": {
          "trusted_ca": { "inline_string": "-----BEGIN CERTIFICATE-----..." },
          "verify_subject_alt_name": ["backend.example.com"]
        },
        "tls_certificates": [{  // Only if upstream requires mTLS
          "certificate_chain": { "..." },
          "private_key": { "..." }
        }]
      },
      "sni": "backend.example.com"
    }
  }
}```


# PCAP

example.pcap taken at ingress gateway, with TLS flow hello_world --> egress --> ingress --> hello_world.

ips
* hello_world 192.168.41.120
* ingress     192.168.212.61
* egress      192.168.41.121


Inpecting pcap and you can see TLS handshakes, with packets marked as 'TLS Record Protocol' 