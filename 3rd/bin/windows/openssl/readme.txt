
 OpenSSL 1.1.0 25 Aug 2016

 Copyright (c) 1998-2016 The OpenSSL Project
 Copyright (c) 1995-1998 Eric A. Young, Tim J. Hudson
 All rights reserved.

 DESCRIPTION
 -----------

 The OpenSSL Project is a collaborative effort to develop a robust,
 commercial-grade, fully featured, and Open Source toolkit implementing the
 Transport Layer Security (TLS) protocols (including SSLv3) as well as a
 full-strength general purpose cryptographic library.

 OpenSSL is descended from the SSLeay library developed by Eric A. Young
 and Tim J. Hudson.  The OpenSSL toolkit is licensed under a dual-license (the
 OpenSSL license plus the SSLeay license), which means that you are free to
 get and use it for commercial and non-commercial purposes as long as you
 fulfill the conditions of both licenses.

 OVERVIEW
 --------

 The OpenSSL toolkit includes:

 libssl (with platform specific naming):
     Provides the client and server-side implementations for SSLv3 and TLS.

 libcrypto (with platform specific naming):
     Provides general cryptographic and X.509 support needed by SSL/TLS but
     not logically part of it.

 openssl:
     A command line tool that can be used for:
        Creation of key parameters
        Creation of X.509 certificates, CSRs and CRLs
        Calculation of message digests
        Encryption and decryption
        SSL/TLS client and server tests
        Handling of S/MIME signed or encrypted mail
        And more...

 INSTALLATION
 ------------

 See the appropriate file:
        INSTALL         Linux, Unix, Windows, OpenVMS, ...
        NOTES.*         INSTALL addendums for different platforms

 SUPPORT
 -------

 See the OpenSSL website www.openssl.org for details on how to obtain
 commercial technical support. Free community support is available through the
 openssl-users email list (see
 https://www.openssl.org/community/mailinglists.html for further details).

 If you have any problems with OpenSSL then please take the following steps
 first:

    - Download the current snapshot from ftp://ftp.openssl.org/snapshot/
      to see if the problem has already been addressed
    - Remove ASM versions of libraries
    - Remove compiler optimisation flags

 If you wish to report a bug then please include the following information in
 any bug report:

    - OpenSSL version: output of 'openssl version -a'
    - Any "Configure" options that you selected during compilation of the
      library if applicable (see INSTALL)
    - OS Name, Version, Hardware platform
    - Compiler Details (name, version)
    - Application Details (name, version)
    - Problem Description (steps that will reproduce the problem, if known)
    - Stack Traceback (if the application dumps core)

 Email the report to:

    rt@openssl.org

 In order to avoid spam, this is a moderated mailing list, and it might
 take a couple of days for the ticket to show up.  (We also scan posts to make
 sure that security disclosures aren't publicly posted by mistake.) Mail
 to this address is recorded in the public RT (request tracker) database
 (see https://www.openssl.org/community/index.html#bugs for details) and
 also forwarded the public openssl-dev mailing list.  Confidential mail
 may be sent to openssl-security@openssl.org (PGP key available from the
 key servers).

 Please do NOT use this for general assistance or support queries.
 Just because something doesn't work the way you expect does not mean it
 is necessarily a bug in OpenSSL. Use the openssl-users email list for this type
 of query.

 You can also make GitHub pull requests. See the CONTRIBUTING file for more
 details.

 HOW TO CONTRIBUTE TO OpenSSL
 ----------------------------

 See CONTRIBUTING

 LEGALITIES
 ----------

 A number of nations, in particular the U.S., restrict the use or export
 of cryptography. If you are potentially subject to such restrictions
 you should seek competent professional legal advice before attempting to
 develop or distribute cryptographic code.
