```
7.A. Anomalous record — zero-MAC Axon advertisement
One device in the dataset broadcasts with address = 00:00:00:00:00:00 while still carrying Axon Service Data (UUID 0xFE6B, registered to Axon Public Safety). RaMBLE positively identifies the device as an
Axon Body Camera / Taser. The user reports the same pattern observed inside a Sydney police-station collection.

Most-likely explanation: the device is in a docked / non-deployed broadcast state in which the AdvA is sanitised but Axon-identifying service data is still emitted. This implies (a) Axon has the technical
capability to broadcast without exposing a stable identifier and (b) NULLWEAR's OUI matcher should be extended in v1.1 to also match on the Axon-specific service-data UUIDs, to provide coverage for this edge case.

The 18-byte service-data payload appears to encode an obfuscated device identifier and a small status word. Whether this constitutes a re-identifiable fingerprint across multiple observations is itself
a privacy question worth follow-up analysis.
```
