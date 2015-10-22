CREATE TABLE supplier (
  sid INT PRIMARY KEY,
  sname VARCHAR(30) NOT NULL,
  status INT NOT NULL,
  city VARCHAR(30) NOT NULL
);

CREATE TABLE part (
  pid INT PRIMARY KEY,
  pname VARCHAR(30) NOT NULL,
  color INT NOT NULL,
  weight REAL NOT NULL,
  city VARCHAR(30) NOT NULL
);

CREATE TABLE shipment (
  sid INT NOT NULL, -- FOREIGN KEY REFERENCES supplier(sid),
  pid INT NOT NULL, -- FOREIGN KEY REFERENCES part(pid),
  qty INT NOT NULL,
  PRIMARY KEY (sid, pid)
);
