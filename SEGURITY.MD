
# Security Policy

## Supported Versions

Security fixes are applied to the latest development version.

## Reporting a Vulnerability

Do not publish exploitable vulnerability details in public issues.

Please use GitHub's private vulnerability reporting feature,
if enabled for this repository.

Include:
- Steps to reproduce
- Expected and actual behavior
- Potential impact
- Relevant logs without secrets or personal data

## Deployment Warning

This project is educational and is not production-ready.

It does not yet implement:
- TLS encryption
- User authentication
- Rate limiting
- Per-client write queues
- Production-grade connection timeouts

Do not expose the server to untrusted networks until these
controls have been implemented and independently tested.
