-- migrate:up
CREATE TABLE workers (
    id SERIAL PRIMARY KEY,
    worker_uuid UUID NOT NULL UNIQUE,
    server_instance_uuid UUID NOT NULL UNIQUE,
    worker_entt_id INT NOT NULL,
    worker_info JSONB NOT NULL
);

-- migrate:down
DROP TABLE IF EXISTS workers;
