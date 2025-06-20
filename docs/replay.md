# Replay report logic

The replay feature was introduced as a simple mechanism to prevent stuck inputs if the last wireless report is lost (since the protocol does not have packet-received confirmation nor any resend logic yet).

It works by re-sending (replaying) the last report of an specific report type several times, and therefore reducing the chances that all these packets are lost.

```mermaid
flowchart LR
    A(["Start"]) --> B{"New input pending to report?"}
        B --> C["YES<br>-Send new report.<br>-Store report.<br>-Reset replay state."] --> B
        B --> D["NO<br>"]
            D --> E{"Enough time passed to consider last report final?"}
                E --> F["YES"]
                    F --> J{"Was the last report replayed enough times?"}
                        J --> K["NO<br>-Replay last report.<br>-Increase replay counter"] --> B
                E --> G["NO"] --> B
```
