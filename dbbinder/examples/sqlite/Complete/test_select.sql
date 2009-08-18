--! use test.yaml   

select
	U."NAME" as "Name",
	F."TITLE" as "Festival",
	F."DATE" as "Date"
from
	"USERS" U join "USERS_FESTIVALS" UF on
		U."IDUSER" = UF."IDUSER"
	join "FESTIVALS" F on
		UF."IDFESTIVAL" = F."IDFESTIVAL"
where
	U."IDUSER" = ? and
	F."DATE" > ?
order by
	F."DATE"