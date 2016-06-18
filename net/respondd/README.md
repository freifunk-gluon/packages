# What is respondd?

## Introduction
respondd is a server distributing information within a network.

For doing so, respondd spawns a UDP socket (in Gluon 1001/udp), optionally joining a multicast group. When a request is received, the information requested is transmitted to the requester.

All information is organized in a non-hierarchical namespace. Each entry identifies a *request name* (e.g. `statistics`, `nodeinfo`, ...) implemented by at least one "provider" C module.
The respond is the result of merging the outputs of all providers for the given request name.

Some providers are implemented in [/package/gluon-respondd](https://github.com/freifunk-gluon/gluon/tree/master).

## Usage
```
respondd [-p <port>] [-g <group> -i <if0> [-i <if1> ..]] [-d <dir>]
  -p <int>         port number to listen on
  -g <ip6>         multicast group, e.g. ff02::2:1001
  -i <string>      interface on which the group is joined
  -d <string>      data provider directory (default: current directory)
  -h               this help
```

## Procotol
Request and response are encoded as byte strings. These strings are sent as UDP packets. Fragmentation is not supported (except by the IP stack), responses are compressed using the *deflate* algorithm.

- The request is the the word '`GET`' followed by any number of request name, separated by spaces.
- The response is a compressed JSON document. The top level object will contain a property for each
  requested name, the rest of the structure is determined by the actual data.
- (Using just a single request name, without '`GET`', as request will return the data uncompressed
  and without an enclosing object. This kind of request is deprecated.)

### Example
Requesting `nodeinfo` as implemented in the Gluon modules.

#### Request
```
GET nodeinfo
```

#### Response (decompressed)
```json
{
  "nodeinfo": {
    "software": {
      "autoupdater": {
        "branch": "stable",
        "enabled": true
      },
      "batman-adv": {
        "version": "2015.1",
        "compat": 15
      },
      "fastd": {
        "version": "v17",
        "enabled": false
      },
      "firmware": {
        "base": "gluon-v2016.1.5-8-gdeac14e",
        "release": "20160615"
      },
      "status-page": {
        "api": 1
      }
    },
    "network": {
      "addresses": [
        "fda0:cab1:e1e5:5116:eade:27ff:fe65:a5af",
        "fe80::eade:27ff:fe65:a5af",
        "2a03:2260:3011:1160:eade:27ff:fe65:a5af"
      ],
      "mesh": {
        "bat0": {
          "interfaces": {
            "wireless": [
              "ea:e3:29:65:a5:af",
              "ea:e3:28:65:a5:af"
            ],
            "other": [
              "ea:df:27:65:a5:af"
            ]
          }
        }
      },
      "mac": "e8:de:27:65:a5:af"
    },
    "location": {
      "latitude": 51.10727924,
      "longitude": 7.00933367
    },
    "owner": {
      "contact": "me@petabyteboy.de"
    },
    "system": {
      "site_code": "lln"
    },
    "node_id": "e8de2765a5af",
    "hostname": "PetaByteBoy",
    "hardware": {
      "model": "TP-Link TL-WDR3600 v1",
      "nproc": 1
    }
  }
}
```

## Implementing modules

respondd providers are C modules (shared objects). These modules should include
\<json-c/json.h> and \<respondd.h>, the latter of which provides the following definitions:

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
