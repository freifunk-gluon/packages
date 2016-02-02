respondd providers are C modules (shared objects). These modules should include
<json-c/json.h> and <respondd.h>, the latter of which provides the following definitions:

        typedef struct json_object * (*respondd_provider)(void);

        struct respondd_provider_info {
                const char *request;
                const respondd_provider provider;
        };

        extern const struct respondd_provider_info respondd_providers[];


The module must define the array `respondd_providers`, e.g. like this:

        static struct json_object * respondd_provider_nodeinfo(void) {
                ...
        }

        static struct json_object * respondd_provider_statistics(void) {
                ...
        }

        const struct respondd_provider_info respondd_providers[] = {
                {"nodeinfo", respondd_provider_nodeinfo},
                {"statistics", respondd_provider_statistics},
                {}
        };

The array must be terminated with an empty entry. The provider for each
request type must return a [JSON-C] JSON object. This JSON object must have exactly 1
reference, and all other memory must be freed by the provider before returning.

The JSON objects returned by different provider modules for the same request type
are merged.

[JSON-C]: https://github.com/json-c/json-c/wiki
