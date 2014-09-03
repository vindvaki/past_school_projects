------------------------------------------------------------------
-- PAYMENT METHODS
------------------------------------------------------------------
insert into t_payment_method values('PayPal');
insert into t_payment_method values('PalPay');

------------------------------------------------------------------
-- USERS
------------------------------------------------------------------

-- Normal user, a buyer but not a seller
insert into t_user(
    user_name, user_id,
    user_password_hash, 
    user_email, user_since,
    user_payment_method, user_payment_details
)
values (
    'Aggi Poki', 'apoki',
    'b25141e4f895e595dbe74e485c71a56f',
    'apoki@email.net', now(),
    'PayPal', 'paypal_user1'
);

-- A seller but not a buyer
insert into t_user(
    user_name, user_id,
    user_password_hash, 
    user_email, user_since,
    seller_info,
    seller_payment_method, seller_payment_details
)
values (
    'Igga Kopi', 'ikopi',
    '1d04becfa989e61fb11729ca45758ae7',
    'ikopi@email.net', now(),
    'Very trustworthy',
    'PalPay', 'palpay_user7'
);

------------------------------------------------------------------
-- AUCTIONS
------------------------------------------------------------------

insert into t_auction(
    auction_seller_id,
    auction_item_label,
    auction_item_description, auction_item_state,
    auction_item_location, auction_item_shipping,
    auction_starting_bid, auction_min_price,
    auction_start, auction_end 
)
values (
    'apoki',
    'grænn hundur',
    'hann er mjög grænn', 2,
    'heima', 0,
    30, 40,
    now()-interval '1 day', now()
);

insert into t_auction(
    auction_seller_id,
    auction_item_label,
    auction_item_description, auction_item_state,
    auction_item_location, auction_item_shipping,
    auction_starting_bid, auction_min_price,
    auction_start, auction_end 
)
values (
    'apoki',
    'ósamhverfur teningur',
    'með óræðan hliðafjölda', 0,
    'undir eldhúsborðinu', 1,
    7, 99,
    now()-interval '111 days', now()-interval '77 days'
);

insert into t_auction(
    auction_seller_id,
    auction_item_label,
    auction_item_description, auction_item_state,
    auction_item_location, auction_item_shipping,
    auction_starting_bid, auction_min_price,
    auction_start, auction_end 
)
values (
    'ikopi',
    'bleikur fíll',
    'hann er líka svolítið gulur', 3,
    'hjá bróður mínum', 2,
    37, 43,
    now()-interval '1 days', now()+interval '11 days'
);

insert into t_auction(
    auction_seller_id,
    auction_item_label,
    auction_item_description, auction_item_state,
    auction_item_location, auction_item_shipping,
    auction_starting_bid, auction_min_price,
    auction_start, auction_end 
)
values (
    'ikopi',
    'tölvuleikur',
    'mjög skemmtilegur', 2,
    'í skúffunni', 2,
    33, 55,
    now()-interval '8 days', now()-interval '7 days'
);



------------------------------------------------------------------
-- CATEGORIES
------------------------------------------------------------------

-- Generted by {c,b,g,r}or{g,b}

insert into t_category values('corg');
insert into t_category values('corb');
insert into t_category values('borg');
insert into t_category values('borb');
insert into t_category values('gorg');
insert into t_category values('gorb');
insert into t_category values('rorg');
insert into t_category values('rorb');
-- two more
insert into t_category values('mehe');
insert into t_category values('ehem');

-- corg
insert into t_auction_category values(1,'corg');
insert into t_auction_category values(2,'corg');
insert into t_auction_category values(3,'corg');
insert into t_auction_category values(4,'corg');

-- corb
insert into t_auction_category values(1,'corb');
insert into t_auction_category values(3,'corb');
insert into t_auction_category values(4,'corb');

-- borg
insert into t_auction_category values(1,'borg');
insert into t_auction_category values(2,'borg');
insert into t_auction_category values(3,'borg');
insert into t_auction_category values(4,'borg');

-- rorg, rorb
insert into t_auction_category values(1,'rorg');
insert into t_auction_category values(1,'rorb');

------------------------------------------------------------------
-- BIDS
------------------------------------------------------------------

-- normal bid
insert into t_bid (
    bid_date, bid_original_date,
    bid_user, bid_auction,
    bid_val, bid_min, bid_max, bid_inc
)
values (
    now()-interval'7.5 days', now()-interval'7.5 days',
    'apoki', 4,
    60, 60, 60, 1
);

-- autobid
insert into t_bid (
    bid_date, bid_original_date,
    bid_user, bid_auction,
    bid_val, bid_min, bid_max, bid_inc
)
values (
    now()-interval'7.5 days',
    now()-interval'7.5 days',
    'apoki', 3,
    37, 37, 83, 7
);

-- contending normal bid
insert into t_bid (
    bid_date, bid_original_date,
    bid_user, bid_auction,
    bid_val, bid_min, bid_max, bid_inc
)
values (
    now()-interval'7.4 days', now()-interval'7.4 days',
    'apoki', 3,
    40, 40, 40, 1
);

-- normal bid
insert into t_bid (
    bid_date, bid_original_date,
    bid_user, bid_auction,
    bid_val, bid_min, bid_max, bid_inc
)
values (
    now()-interval'93 days', now()-interval'93 days',
    'ikopi', 2,
    400, 400, 400, 33
);

------------------------------------------------------------------
-- DEALS and REVIEWS
------------------------------------------------------------------

insert into t_deal (
    user_id, auction_id, deal_val, deal_date,
    seller_rating, seller_review, seller_review_date,
    buyer_rating, buyer_review,buyer_review_date
)
values (
    'apoki', 4, 60, now()-interval'6 days',
    9, 'Allt gekk vel en þetta var leiðinlegur tölvuleikur',now(),
    10, 'Frábær viðskiptavinur',now()
);

insert into t_deal (
    user_id, auction_id, deal_val, deal_date,
    seller_rating, seller_review, seller_review_date,
    buyer_rating, buyer_review,buyer_review_date
)
values (
    'ikopi', 2, 400, now()-interval'76 days',
    7, 'Lyktar illa', now(),
    3, 'Ég skil ekkert í þessum tening!', now()
);
