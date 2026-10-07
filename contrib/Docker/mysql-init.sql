-- Container variant of sql/create/create_mysql.sql: the servers connect
-- from other containers, so the user is created for any host.
CREATE USER 'arguscore'@'%' IDENTIFIED BY 'arguscore' WITH MAX_QUERIES_PER_HOUR 0 MAX_CONNECTIONS_PER_HOUR 0 MAX_UPDATES_PER_HOUR 0;

GRANT USAGE ON * . * TO 'arguscore'@'%';

CREATE DATABASE `world` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

CREATE DATABASE `characters` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

CREATE DATABASE `auth` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

CREATE DATABASE `hotfixes` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

GRANT ALL PRIVILEGES ON `world` . * TO 'arguscore'@'%' WITH GRANT OPTION;

GRANT ALL PRIVILEGES ON `characters` . * TO 'arguscore'@'%' WITH GRANT OPTION;

GRANT ALL PRIVILEGES ON `auth` . * TO 'arguscore'@'%' WITH GRANT OPTION;

GRANT ALL PRIVILEGES ON `hotfixes` . * TO 'arguscore'@'%' WITH GRANT OPTION;
