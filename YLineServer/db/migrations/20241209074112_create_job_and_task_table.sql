-- migrate:up
-- we only store completed, pending and failed status in database
-- rest of the status are stored in redis
CREATE TYPE exec_status AS ENUM ('completed', 'failed', 'pending');
CREATE TABLE job (
    id SERIAL PRIMARY KEY,
    job_name VARCHAR(255) NOT NULL,
    status exec_status NOT NULL DEFAULT 'pending',
    created_at TIMESTAMPTZ DEFAULT NOW(),
    submit_user VARCHAR(255) NOT NULL,

    FOREIGN KEY (submit_user) REFERENCES users(username)
);

CREATE TABLE task (
    task_id VARCHAR(255) not NULL,
    task_order INT NOT NULL,
    task_name VARCHAR(255) NOT NULL,
    job_id INT NOT NULL,
    dependency BOOLEAN NOT NULL DEFAULT FALSE,
    status exec_status NOT NULL DEFAULT 'pending',

    PRIMARY KEY (task_id, job_id),
    FOREIGN KEY (job_id) REFERENCES job(id)
);
-- migrate:down

-- Drop the task table
DROP TABLE IF EXISTS task;

-- Drop the job table
DROP TABLE IF EXISTS job;

-- Drop the exec_status type
DROP TYPE IF EXISTS exec_status;
