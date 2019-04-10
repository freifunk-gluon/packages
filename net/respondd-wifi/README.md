This module adds a respondd wifi usage statistics provider.
The format is the following:

```json
{
  "statistics": {
    "clients":{
      "wifi24": 3,
      "wifi5": 7
    },
  },
  "neighbours": {
    "wifi":{
      "00:11:22:33:44:55:66":{
        "frequency": 5220,
        "neighbours":{
          "33:22:33:11:22:44":{
            "signal": 191,
            "inactive": 50
          }
        }
      }
    }
  }
}
```
