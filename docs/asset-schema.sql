-- =====================================================================
-- Project NULLWEAR — Asset Register Schema
-- =====================================================================
--
-- Reference SQLite schema for an agency-side asset register that tracks
-- NULLWEAR units across their lifecycle. This is a minimum-viable schema;
-- agencies with existing asset-management systems should integrate by
-- mapping these fields into their existing schema.
--
-- See docs/09-operations-manual.md §2 for the operational use of this
-- register.
--
-- IMPORTANT: rows in `unit_issuance` link a NULLWEAR serial to an
-- officer badge. Treat this database as PROTECTED-classified data
-- (Australian PSPF) or equivalent. See docs/14-security-considerations.md
-- and docs/16-secrets-and-publishing-policy.md.
--
-- License: MIT
-- =====================================================================

-- ---------------------------------------------------------------------
-- Reference data
-- ---------------------------------------------------------------------

CREATE TABLE IF NOT EXISTS variant (
    code        TEXT PRIMARY KEY,
    description TEXT NOT NULL
);

INSERT OR IGNORE INTO variant (code, description) VALUES
    ('NULLWEAR/P', 'Personal-issue body-worn device'),
    ('NULLWEAR/V', 'Vehicle-mounted device'),
    ('NULLWEAR/S', 'Station-mounted distributed-antenna unit');


CREATE TABLE IF NOT EXISTS unit_status (
    code        TEXT PRIMARY KEY,
    description TEXT NOT NULL
);

INSERT OR IGNORE INTO unit_status (code, description) VALUES
    ('InStock',    'Received from CM, tested, available for issue'),
    ('Charging',   'On dock, not currently issued'),
    ('Issued',     'Issued to an officer'),
    ('FaultRMA',   'Returned with reported fault, awaiting depot diagnosis'),
    ('Refurbish',  'Returned to CM for refurbishment'),
    ('Lost',       'Reported lost'),
    ('Stolen',     'Reported stolen'),
    ('EndOfLife',  'Battery wear past threshold; awaiting recycling');


-- ---------------------------------------------------------------------
-- Per-unit lifecycle
-- ---------------------------------------------------------------------

CREATE TABLE IF NOT EXISTS unit (
    serial            TEXT PRIMARY KEY,             -- laser-etched on enclosure
    variant           TEXT NOT NULL REFERENCES variant(code),
    mfg_batch         TEXT,
    mfg_date          DATE,
    received_date     DATE NOT NULL,
    firmware_revision TEXT,                          -- e.g. "1.0.0+abcdef0"
    current_status    TEXT NOT NULL REFERENCES unit_status(code),
    last_status_change TIMESTAMP NOT NULL DEFAULT (datetime('now')),
    notes             TEXT
);


CREATE TABLE IF NOT EXISTS unit_status_history (
    id                INTEGER PRIMARY KEY AUTOINCREMENT,
    serial            TEXT NOT NULL REFERENCES unit(serial),
    new_status        TEXT NOT NULL REFERENCES unit_status(code),
    changed_at        TIMESTAMP NOT NULL DEFAULT (datetime('now')),
    changed_by        TEXT,                          -- depot tech identifier
    reason            TEXT
);

CREATE INDEX IF NOT EXISTS idx_status_history_serial
    ON unit_status_history(serial);


-- ---------------------------------------------------------------------
-- Issuance — the operationally-sensitive table
-- ---------------------------------------------------------------------
--
-- This table links a NULLWEAR serial to an officer badge number.
-- It is the single most sensitive operational data set in the project.
-- Treat as PROTECTED. Audit access. Encrypt at rest.

CREATE TABLE IF NOT EXISTS unit_issuance (
    id                INTEGER PRIMARY KEY AUTOINCREMENT,
    serial            TEXT NOT NULL REFERENCES unit(serial),
    officer_badge     TEXT NOT NULL,                 -- agency-internal badge id
    issued_at         TIMESTAMP NOT NULL DEFAULT (datetime('now')),
    returned_at       TIMESTAMP,
    issued_by         TEXT,                          -- quartermaster id
    returned_to       TEXT,
    notes             TEXT
);

CREATE INDEX IF NOT EXISTS idx_issuance_serial
    ON unit_issuance(serial);
CREATE INDEX IF NOT EXISTS idx_issuance_officer
    ON unit_issuance(officer_badge);


-- ---------------------------------------------------------------------
-- Test results
-- ---------------------------------------------------------------------

CREATE TABLE IF NOT EXISTS atp_run (
    id                INTEGER PRIMARY KEY AUTOINCREMENT,
    serial            TEXT NOT NULL REFERENCES unit(serial),
    bench_id          TEXT,
    technician        TEXT,
    started_at        TIMESTAMP NOT NULL,
    finished_at       TIMESTAMP NOT NULL,
    overall_verdict   TEXT NOT NULL,                 -- PASS / FAIL / MARGINAL / INDETERMINATE
    report_json       TEXT NOT NULL,                 -- full JSON per atp_schema.json
    schema_version    TEXT NOT NULL DEFAULT '1.0.0'
);

CREATE INDEX IF NOT EXISTS idx_atp_serial ON atp_run(serial);
CREATE INDEX IF NOT EXISTS idx_atp_verdict ON atp_run(overall_verdict);


CREATE TABLE IF NOT EXISTS field_test_run (
    id                INTEGER PRIMARY KEY AUTOINCREMENT,
    serial            TEXT NOT NULL REFERENCES unit(serial),
    test_loop_id      TEXT,
    technician        TEXT,
    started_at        TIMESTAMP NOT NULL,
    finished_at       TIMESTAMP NOT NULL,
    par_5m_vertical   REAL,
    par_5m_pocket     REAL,
    par_indoor        REAL,
    par_outdoor       REAL,
    overall_verdict   TEXT NOT NULL,
    report_json       TEXT
);

CREATE INDEX IF NOT EXISTS idx_field_serial ON field_test_run(serial);


-- ---------------------------------------------------------------------
-- Charging / fault telemetry
-- ---------------------------------------------------------------------

CREATE TABLE IF NOT EXISTS dock_event (
    id                INTEGER PRIMARY KEY AUTOINCREMENT,
    serial            TEXT NOT NULL REFERENCES unit(serial),
    event_type        TEXT NOT NULL,                 -- 'docked', 'undocked', 'charged', 'fault'
    event_at          TIMESTAMP NOT NULL DEFAULT (datetime('now')),
    soc_percent       INTEGER,
    cell_voltage_mv   INTEGER,
    fault_code        TEXT
);

CREATE INDEX IF NOT EXISTS idx_dock_serial ON dock_event(serial);
CREATE INDEX IF NOT EXISTS idx_dock_at ON dock_event(event_at);


-- ---------------------------------------------------------------------
-- Aggregate views for daily reporting
-- ---------------------------------------------------------------------

CREATE VIEW IF NOT EXISTS v_fleet_summary AS
SELECT
    variant,
    current_status,
    COUNT(*) AS count
FROM unit
GROUP BY variant, current_status
ORDER BY variant, current_status;


CREATE VIEW IF NOT EXISTS v_units_in_service AS
SELECT
    u.serial,
    u.variant,
    u.firmware_revision,
    i.officer_badge,
    i.issued_at,
    julianday('now') - julianday(i.issued_at) AS days_in_service
FROM unit u
JOIN unit_issuance i ON i.serial = u.serial AND i.returned_at IS NULL
WHERE u.current_status = 'Issued';


CREATE VIEW IF NOT EXISTS v_atp_pass_rate_30d AS
SELECT
    COUNT(*) AS total_atp_runs,
    SUM(CASE WHEN overall_verdict = 'PASS' THEN 1 ELSE 0 END) AS passes,
    ROUND(100.0 * SUM(CASE WHEN overall_verdict = 'PASS' THEN 1 ELSE 0 END) / COUNT(*), 1) AS pass_rate_pct
FROM atp_run
WHERE started_at >= datetime('now', '-30 days');


-- ---------------------------------------------------------------------
-- Suggested triggers (optional)
-- ---------------------------------------------------------------------

-- Auto-record any status change in the history table
CREATE TRIGGER IF NOT EXISTS trg_unit_status_change
AFTER UPDATE OF current_status ON unit
WHEN OLD.current_status != NEW.current_status
BEGIN
    INSERT INTO unit_status_history (serial, new_status, reason)
    VALUES (NEW.serial, NEW.current_status, 'auto-trigger');
    UPDATE unit SET last_status_change = datetime('now') WHERE serial = NEW.serial;
END;


-- =====================================================================
-- End of schema
-- =====================================================================
