-- Opin tilboð notanda 'u_id'
create or replace function user_open_bids(u_id varchar(20))
returns table(
    bid_id integer, auction_id integer, 
    auction_item_label varchar(30),
    bid_original_date timestamp, bid_date timestamp, auction_end timestamp,
    bid_val integer, 
    bid_min integer, bid_max integer, bid_inc integer
) as $$
begin
    return query 
        select 
            B.bid_id, A.auction_id, 
            A.auction_item_label,
            B.bid_original_date, B.bid_date, A.auction_end,
            B.bid_val, B.bid_min, B.bid_max, B.bid_inc
        from t_bid B, t_auction A
        where B.bid_auction = A.auction_id
            and B.bid_user = u_id
            and A.auction_start < now()
            and A.auction_end > now();
end;
$$ language plpgsql;

-- allir flokkar
create or replace function get_all_categories()
returns setof t_category as $$
begin
    return query select * from t_category;
end;
$$ language plpgsql;

-- Þau 'n' uppboð, sem er styst síðan byrjuðu og eru enn í gangi
create or replace function get_recent_auctions(n integer)
returns setof t_auction as $$
begin
    return query
        select * from t_auction
        where auction_start >= now()
            and auction_end < now()
        order by auction_start desc
        limit n;
end;
$$ language plpgsql;

-- Allir hlutir í tilteknum flokki
create or replace function items_in_category(c_id varchar(60))
returns setof t_auction as $$
begin
    return query
        select * from t_auction
        where t_auction.auction_id in (
            select AC.auction_id from t_auction_category AC
            where AC.category_id = c_id
        );
end;
$$ language plpgsql;
