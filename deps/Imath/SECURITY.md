<!-- SPDX-License-Identifier: BSD-3-Clause -->
<!-- Copyright (c) Contributors to the OpenEXR Project -->

# Security Policy

## Reporting a Vulnerability

If you think you've found a potential vulnerability in Imath, please
report it by filing a GitHub [security
advisory](https://github.com/AcademySoftwareFoundation/Imath/security/advisories/new). Alternatively,
email security@openexr.com and provide your contact info for further
private/secure discussion.  If your email does not receive a prompt
acknowledgement, your address may be blocked.

Our policy is to acknowledge the receipt of vulnerability reports
within 48 hours. Our policy is to address critical security vulnerabilities
rapidly and post patches within 14 days if possible.

## Supported Versions

This gives guidance about which branches are supported with patches to
security vulnerabilities.

| Version / branch  | Supported                                            |
| --------- | ---------------------------------------------------- |
| main      | :white_check_mark: :construction: ALL fixes immediately, but this is a branch under development with a frequently unstable ABI and occasionally unstable API. |
| 3.1.x    | :white_check_mark: All fixes that can be backported without breaking ABI compatibility. |
| 3.0.x    | :warning: Only the most critical fixes, only if they can be easily backported. |

## Signed Releases

Releases artifacts are signed via
[sigstore](https://www.sigstore.dev). See
[release-sign.yml](.github/workflows/release-sign.yml) for details.

To verify a downloaded release at a given tag:

    % pip install sigstore
    % sigstore verify github --cert-identity https://github.com/AcademySoftwareFoundation/Imath/.github/workflows/release-sign.yml@refs/tags/<tag> Imath-<tag>.tar.gz

## Security Expectations

### Software Features

- The Imath project implements basic vector, matrix, and math
  operations, and is used throughout the motion picture industry and
  beyond, on Linux, macOS, and Windows.

- The project consists of a software run-time library, implemented in
  C/C++ and built via cmake. The project also distributes python
  wrappings for the C/C++ I/O API.

- The library provides no external input/output. 

- Other than the website and online technical documentation, the
  project implements no web/online services or network communication
  protocols.  The library never requests any security or
  authentication credentials or login information from users.

  The website implements no interactive features and requires no login
  credentials.

### Software Dependencies

Imath has no external dependencies.

The Imath python bindings depend on python and boost.

### Development Cycle and Distribution

Imath is downloadable and buildable by C/C++ source via GitHub. Only
members of the project's Technical Steering Committee, all veteran
software engineers at major motion picture studios or vendors, have
write permissions on the source code repository. All critical software
changes are reviewed by multiple TSC members.

The library is distributed in binary form via many common package
managers across all platforms.



