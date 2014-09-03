-- Dæmi 4

-- (a) Innskráningarsíða

create or replace function user_login(
    u_id varchar(20),
    pw_hash varchar(30))
returns boolean as $$
begin
    return exists (
        select * from t_user 
        where user_id = u_id and user_password_hash = pw_hash
    );
end;
$$ language plpgsql;


-- (b) Upphafsíða notanda

create or replace function user_open_bids(u_id varchar(20))
returns table(
    bid_id varchar(20),
    auction_id integer, 
    auction_item_label varchar(30),
    bid_original_date date,
    bid_date date, 
    auction_end date,
    bid_val integer,
    bid_min integer,
    bid_max integer,
    bid_val integer
) as $$
begin
    return query 
    select 
        bid_id,
        auction_id, 
        auction_item_label,
        bid_original_date,
        bid_date, 
        auction_end,
        bid_val,
        bid_min,
        bid_max,
        bid_val
    from t_bid join t_auction
        on bid_auction = auction_id
    where auction_end > now();
end;
$$ language plpgsql;

create or replace function get_all_categories() returns setof t_category as $$
begin
    return query select * from t_category;
end;
$$ language plpgsql;

-- þau 'n' uppboð sem er styst síðan byrjuðu
create or replace function get_recent_auctions(n integer)
returns setof t_auction as $$
begin
    return query 
        select * from t_auction
        where auction_start >= now()
        order by auction_start desc
        limit n;
end;
$$ language plpgsql;


-- (c)  Uppboðssíða hlutar

create or replace function get_auction_page(id integer) 
returns table(
    auction_id integer,
    auction_seller_id varchar(20),
    auction_item_label varchar(30),
    auction_item_description varchar(10000),
    auction_item_state integer,
    auction_item_location varchar(1000),
    auction_item_shipping integer,
    auction_starting_bid integer,
    auction_start date,
    auction_end date,

    time_left interval,
    seller_deals integer,
    seller_reputation numeric
) as $$
begin
    return query 
        select 
            auction_id,
            auction_seller_id,
            auction_item_label,
            auction_item_description,
            auction_item_state,
            auction_item_location,
            auction_item_shipping,
            auction_starting_bid,
            auction_start,
            auction_end,
            auction_end - now() as time_left,
            -- TODO: deals and reputation
        from t_auction
        where auction_id = id
end;
$$ language plpgsql;


-- (d) Tilboð í hlut

-- Ath. autobid_raise trigger sem ákvarðar hæsta boð að hverju sinni.

create or replace function new_bid(
    user_id varchar(20), auction_id integer, 
    bmin integer, bmax integer, binc integer
) returns void as $$
declare
    newdate date;
begin
    newdate := now();
    insert into t_bid(
        bid_date, bid_original_date,
        bid_user, bid_auction,
        bid_val, bid_min, bid_max, bid_inc
    )
    values (
        newdate, newdate,
        user_id, auction_id,
        bmin, bmin, bmax, binc
    );
    return;
end;
$$ language plpgsql;


-- (e) Einkunnagjöf

-- Seljandi gefur kaupanda einkunn fyrir viðskipti 
create or replace function review_buyer(
    d_id integer, rating integer, review varchar(1000)
) returns void as $$
begin
    update t_deal
        set buyer_rating = rating,
            buyer_review = review
        where deal_id = d_id;
    return;
end;
$$ language plpgsql;

-- Kaupandi gefur seljanda einkunn fyrir viðskipti
create or replace function review_seller(
    d_id integer, rating integer, review varchar(1000)
) returns void as $$
begin
    update t_deal
        set seller_rating = rating,
            seller_review = review
        where deal_id = d_id;
    return;
end;
$$ language plpgsql;


-- (f) Upplýsingasíða seljanda
-- Hve lengi seljandi hefur verið skráður
-- Meðaleinkunn
-- Síðustu 5 umsagnir um seljandann
-- Eitthvað fleira

-- Venjulegar upplýsingar fást með því að sækja notandann með
-- þetta id

-- Einkunnir og athugasemdir fást með því að skoða kaupsamninga sem seljandi
-- hefur gert.

-- (g) Setja inn nýjan notanda
create or replace function add_user(
    u_name varchar(50),
    u_id varchar(20),
    u_pw_hash varchar(32),
    u_email varchar(30)
) returns void as $$
begin
    insert into t_user(user_name, user_id, user_password_hash, 
        user_email, user_since)
    values (u_name, u_id, u_pw_hash, u_email, now());
end;
$$ language plpgsql;

-- (h) Setja inn nýjan seljanda

-- Skilyrðið er að seljandi sé þegar orðinn notandi
create or replace function update_seller_info(
    u_id varchar(20),
    s_info text,
    s_pay varchar(20),
    s_pay_details varchar(1000)
) returns void as $$
begin
    update t_user
        set seller_info = s_info,
            seller_payment_method = s_pay,
            seller_payment_details = s_pay_details
        where user_id = u_id;
    return;
end;
$$ language plpgsql;
