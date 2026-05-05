# Bugs and Issues detected and rectified

#	Bug	Severity	Fix
1	RADIO_0_IRQHandler — name is wrong for nRF5340 cpunet (it's RADIO_IRQHandler)	Silent-failure critical — handler never fires	Use IRQ_DIRECT_CONNECT(RADIO_IRQn, …)
2	Same for TIMER0_IRQHandler	Critical	Use IRQ_CONNECT(TIMER1_IRQn, …)
3	Defining a bare void XXX_IRQHandler(void) doesn't work in Zephyr — you must IRQ_CONNECT	Critical	Same fix
4	TIMER0 conflicts with Zephyr's tickless kernel	Critical	Move hop timer to TIMER1
5	nrfx_dppi_channel_alloc() API takes an instance pointer in nrfx ≥ 3.0 (NCS 2.5+)	Compile failure	Either fix to instance form or drop DPPI entirely
6	DPPI link DISABLED→TXEN is unconditional — fires after RX-end too, causing accidental TX	Logic bug	Drop DPPI; use synchronous spin-wait pattern (~10 µs added latency, well within budget)
7	radio_arm_rx() doesn't restore SHORTS register from TX mode → next RX won't start BCMATCH	Logic bug — first phase doesn't trigger	Restore SHORTS in arm_rx
8	radio_arm_rx() doesn't clear stale EVENTS_BCMATCH	Race risk	Clear before re-arm
9	TIMER ISR has spin-wait + then re-arms; collides with DISABLED ISR doing same	Race	Let DISABLED ISR be the only re-arm path
10	Two DPPI channels allocated and never used	Minor — wastes resource	Remove
