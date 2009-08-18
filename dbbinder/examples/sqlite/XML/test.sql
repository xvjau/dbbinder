create table "USERS"
(
	"IDUSER"	integer not null primary key,
	"NAME"		text,
	"LOGIN"		text,
	"PASSWORD"	text
);

insert into "USERS" values(1,'Aphrodite','aphrodite@sex.gr','ganymede');
insert into "USERS" values(2,'Venus','venus@sky.com','titan');
insert into "USERS" values(3,'Athena','athena@crea.org.br','222222');
insert into "USERS" values(4,'Ceres','ceres@agriculture.europa.eu','quirinalis');

create table "FESTIVALS"
(
	"IDFESTIVAL"	integer not null primary key,
	"TITLE"			text,
	"LOCATION"		text,
	"DATE"			integer
);

insert into "FESTIVALS" values (1,'Aphrodisiac','Athina',20987);
insert into "FESTIVALS" values (2,'Ambarvalia','Colle Aventino',38342);

create table "USERS_FESTIVALS"
(
	"IDUSER"		integer not null,
	"IDFESTIVAL"	integer not null,
	primary key("IDUSER", "IDFESTIVAL")
);

insert into "USERS_FESTIVALS" values (1, 1);
insert into "USERS_FESTIVALS" values (2, 1);
insert into "USERS_FESTIVALS" values (3, 1);
insert into "USERS_FESTIVALS" values (4, 1);
insert into "USERS_FESTIVALS" values (2, 2);
insert into "USERS_FESTIVALS" values (3, 2);
insert into "USERS_FESTIVALS" values (4, 2);
