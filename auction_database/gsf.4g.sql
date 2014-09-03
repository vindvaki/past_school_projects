create or replace function add_user(
    u_id varchar(20), u_pw_hash varchar(32),
    u_name varchar(50), u_email varchar(30)
) returns void as $$
begin
    insert into t_user(user_id, user_password_hash, 
        user_name, user_email, user_since)
    values (u_name, u_id, u_pw_hash, u_email, now());
end;
$$ language plpgsql;
