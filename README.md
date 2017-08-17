# RPC Service - WiFi

## Wifi.Scan

Scan wifi networks.

Arguments: none.

Example usage:

<pre class="command-line language-bash" data-user="chris" data-host="localhost" data-output="2-100"><code>mos call Wifi.Scan
{
  "results": [
    {
      "ssid": "my_essid",
      "auth": 0,
      "channel": 1,
      "rssi": -25
    },
    ...
  ]
}</code></pre>
