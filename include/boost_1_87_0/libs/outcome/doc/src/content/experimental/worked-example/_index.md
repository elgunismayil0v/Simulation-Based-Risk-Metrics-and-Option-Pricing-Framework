+++
title = "Worked example: Custom domain (the short way)"
weight = 80
+++

The section after this one will take the long way through defining a custom domain
which sometimes is necessary if you have particularly bespoke requirements.
If however you just want to wrap a custom `enum` type of yours into its
own custom code domain, the boilerplate can be automated away
by filling in a few simple fields like this:

{{% snippet "quick_status_code_from_enum.cpp" "preamble" %}}

Here we supply the bare minimum requirements for a status code domain:

1. The name in text, so it can be printed.
2. The unique UUID to identify when multiple copies of the domain are the same.
PLEASE use https://www.random.org/cgi-bin/randbyte?nbytes=16&format=h to generate
this, do not twiddle a few bits.
3. For each enum value, its printable text and a sequence of `errc::` enumerations
which you think it is semantically equivalent to i.e. its mapping onto `generic_code`
which is how status code defines the common mapping between status codes. If you later compare the
status code to one of those values (or to another status code which also provides
a mapping), if the generic codes are equivalent then the comparison will return true.
This means code like `if(sc == errc::no_such_file_or_directory) ...` would match
all custom error codes which mean 'something was not found'.
4. OPTIONAL: if you would like the custom status code type generated by this
to have additional member functions or additional payload, you can define a mixin
here to inject either data or functions or both. If you omit this, nothing gets
injected.