-- migrate:up
ALTER TABLE job RENAME TO jobs;
ALTER TABLE task RENAME TO tasks;


-- migrate:down
ALTER TABLE jobs RENAME TO job;
ALTER TABLE tasks RENAME TO task;
