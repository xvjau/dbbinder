--! use postgres.yaml
--! name select_blob

select
    id,
    name
from
    testdata
where
    name != $1::varchar
