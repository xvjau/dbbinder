--! use postgres.yaml
--! name select_blob
--! param name string

select
    id,
    name
from
    testdata
where
    name != $1::varchar
