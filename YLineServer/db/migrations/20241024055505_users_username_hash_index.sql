-- migrate:up
CREATE INDEX idx_users_username_hash ON users USING hash (username);

-- migrate:down
DROP INDEX IF EXISTS idx_users_username_hash;
