--! use postgres.yaml
--! name select_blob
--! param name string

select
  "id",
  "name",
  "DateTest",
  "TimeZTest",
  "TimeTest",
  "TimestampZTest",
  "TimestampTest",
  "TextTest"
from
  "testdata"
where
    "name" != $1::varchar
