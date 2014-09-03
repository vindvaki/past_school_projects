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
end; $$ language plpgsql;
