This module adds a respondd wifi usage statistics provider.
The format is the following:

```json
{
  "statistics": {
    "clients":{
      "wifi24": 3,
      "wifi5": 7
    },
    "wireless": [
      {
        "frequency": 5220,
        "txpower": 1700,
        "active": 366561161,
        "busy": 46496566,
        "rx": 808415,
        "tx": 41711344,
        "noise": 162,
        "clients": 5
      },
      {
        "frequency": 5220,
        "txpower": 1700,
        "active": 366561161,
        "busy": 46496566,
        "rx": 808415,
        "tx": 41711344,
        "noise": 162,
        "clients": 2
      },
      {
        "frequency": 2437,
        "txpower": 2000,
        "active": 366649704,
        "busy": 205221222,
        "rx": 108121446,
        "tx": 85453679,
        "noise": 161,
        "clients": 3
      }
    ]
  }
}
```

The numbers `active`, `busy`, `rx` and `tx` are times in milliseconds, where
`busy`, `rx` and `tx` have to be interpreted by taking the quotient with
`active`.

The motivation for having a list with the frequency as a value in the objects
instead of having an object with the frequency as keys is that multiple wifi
devices might be present, in which case the same frequency can appear multiple
times (because the statistics are reported once for every phy).
