# SAR: Mapping

Add SAR timing to your map!

## Start & Stop Triggers

- Place a `point_servercommand` object anywhere in the map
- Use two `trigger_multiple` objects for start and stop
  - Add `OnStartTouch` as output
  - Target the created servercommand object
  - Select `Command` as input
  - Use `sar_timer_start` as parameter
  - Do the same for the second trigger with `sar_timer_stop`

![start.png](images/start.png)

## Using Checkpoints

- Use `trigger_multiple` object for start
  - Trigger `sar_timer_start`
  - Trigger `sar_cps_clear`
  - Enable checkpoint object
- Use `trigger_multiple` object for checkpoint
  - Trigger `sar_cps_add`
  - Disable itself
- Use `trigger_multiple` object for stop
  - Trigger `sar_timer_stop`

![cpstart.png](images/cpstart.png)
![cp1.png](images/cp1.png)
