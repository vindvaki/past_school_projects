-- Tími skráður og meðaleinkunn seljanda
create or replace function get_seller_stats(s_id varchar(20))
returns setof seller_stats as $$
begin
    return query
        select * from seller_stats
        where user_id = s_id;
end;
$$ language plpgsql;

-- 'n' nýjustu umsagnirnar
create or replace function latest_seller_reviews(
   s_id varchar(20), n integer
) returns table ( rating integer, review text ) as $$
begin
    return query
        select seller_rating, seller_review
        from t_deal D
        where D.auction_id in (
            select A.auction_id from t_auction A
            where A.auction_seller_id = s_id
        ) order by D.seller_review_date desc
        limit n;
end;
$$ language plpgsql;
