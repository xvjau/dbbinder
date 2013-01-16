--! use blob_test.yaml
--! name insert_blob
--! param comment blob

insert into
    dbtest (`comment`)
values
    (?)
