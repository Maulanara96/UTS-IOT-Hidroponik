CREATE DATABASE IF NOT EXISTS hidroponik_db 
DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;


USE hidroponik_db;


CREATE TABLE IF NOT EXISTS data_sensor (
  id INT PRIMARY KEY,
  suhu FLOAT,
  humidity FLOAT,
  lux FLOAT,
  timestamp DATETIME
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;


INSERT INTO data_sensor (id, suhu, humidity, lux, timestamp) VALUES
(101, 36, 36, 25, '2010-09-18 07:23:48'),
(226, 36, 36, 27, '2011-05-02 12:29:34'),
(227, 21, 70, 150, '2011-05-03 10:00:00'),
(228, 25, 65, 200, '2011-05-03 11:00:00'),
(229, 30, 60, 300, '2011-05-03 12:00:00');