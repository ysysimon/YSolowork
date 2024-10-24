-- migrate:up
-- 为 worker_uuid 字段创建哈希索引
CREATE INDEX idx_worker_uuid_hash ON workers USING hash (worker_uuid);
-- 为 server_instance_uuid 字段创建哈希索引
CREATE INDEX idx_server_instance_uuid_hash ON workers USING hash (server_instance_uuid);

-- migrate:down
-- 回滚时删除 worker_uuid 和 server_instance_uuid 的哈希索引
DROP INDEX IF EXISTS idx_worker_uuid_hash;
DROP INDEX IF EXISTS idx_server_instance_uuid_hash;