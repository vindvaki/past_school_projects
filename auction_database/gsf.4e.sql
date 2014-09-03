-- Seljandi gefur kaupanda einkunn og umsögn fyrir viðskipti 
create or replace function review_buyer(
    d_id integer, rating integer, review text
) returns void as $$
begin
    update t_deal
        set buyer_rating = rating,
            buyer_review = review
        where deal_id = d_id;
    return;
end;
$$ language plpgsql;

-- Kaupandi gefur seljanda einkunn og umsögn fyrir viðskipti
create or replace function review_seller(
    d_id integer, rating integer, review text
) returns void as $$
begin
    update t_deal
        set seller_rating = rating,
            seller_review = review
        where deal_id = d_id;
    return;
end;
$$ language plpgsql;

