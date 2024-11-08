-- migrate:up
-- 删除 server_instance_uuid 列的 UNIQUE 约束
ALTER TABLE workers DROP CONSTRAINT IF EXISTS workers_server_instance_uuid_key;

-- migrate:down
-- 如果需要恢复 UNIQUE 约束，可以在此处重新添加
ALTER TABLE workers ADD CONSTRAINT workers_server_instance_uuid_key UNIQUE (server_instance_uuid);
