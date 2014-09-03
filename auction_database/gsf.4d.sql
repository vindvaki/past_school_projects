-- Nýtt tilboð í hlut
create or replace function new_bid(
    user_id varchar(20), auction_id integer,
    bmin integer, bmax integer, binc integer
) returns void as $$
declare
    newdate timestamp;
begin
    newdate := now();
    insert into t_bid values(
        newdate, newdate,
        user_id, auction_id,
        bmin, bmin, bmax, binc
    );
    return;
end;
$$ language plpgsql;
