This is a collection of different ways to do dns lookups in C++. 

Verify using 'dig' command, NOT nslookup as it doesn't use the same system apis. eg: dig www.google.co.in


------------------------------------------------------------------------
DNS Data Records
------------------------------------------------------------------------
DNS database holds DNS resource records (RR) in it's rows. RR format is, 
{Name, TTL (optional), Class (usual IN), Type, DataLength, Data}

https://techdocs.f5.com/kb/en-us/archived_products/3-dns/manuals/product/3dns4_5ref/3dns_resourcerecs.html#:~:text=A%20resource%20record%20is%20an,domain%20name%20system%20(DNS).&text=The%20first%20field%2C%20name%2C%20is,always%20start%20in%20column%201.

Type of Record                              Description
A     (Address)                             Maps host names to IP addresses.
CNAME (Canonical Name)                      Defines a host alias.
MX    (Mail Exchange)                       Identifies where to send mail for a given domain name.
NS    (Name Server)                         Identifies the name servers for a domain.
PTR   (Pointer)                             Maps IP addresses to host names.
SOA   (Start of Authority)                  Indicates that a name server is the best source of information for a zone's data


------------------------------------------------------------------------