create or replace view seller_stats as
    select 
        S.user_id,
        age(S.user_since),
        count(D.deal_id),
        avg(D.seller_rating)
    from t_user S left outer join t_deal D
        on S.user_id in (
            select A.auction_seller_id
            from t_auction A
            where A.auction_id = D.auction_id
        ) group by S.user_id, S.user_since;

create or replace function get_auction_page(a_id integer) 
returns table(
    -- standard auction info
    auction_id integer,
    auction_seller_id varchar(20),
    auction_item_label varchar(30),
    auction_item_description text,
    auction_item_state integer,
    auction_item_location varchar(1000),
    auction_item_shipping integer,
    auction_starting_bid integer,
    auction_start timestamp,
    auction_end timestamp,
    -- calculated columns
    time_left interval,
    count bigint,
    avg numeric
) as $$
begin
    return query 
        select 
            -- standard
            A.auction_id,
            A.auction_seller_id,
            A.auction_item_label,
            A.auction_item_description,
            A.auction_item_state,
            A.auction_item_location,
            A.auction_item_shipping,
            A.auction_starting_bid,
            A.auction_start,
            A.auction_end,
            -- calculate
            A.auction_end - now(),
            S.count,
            S.avg
        from t_auction A, seller_stats S
            where A.auction_seller_id = S.user_id
                and A.auction_id = a_id;
end;
$$ language plpgsql;
