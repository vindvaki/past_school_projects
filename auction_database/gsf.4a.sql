create or replace function authenticate_user (
    u_id varchar(20), pw_hash char(32)
) returns boolean as $$
begin
    return exists (
        select * from t_user
        where user_id = u_id and user_password_hash = pw_hash
    );
end;
$$ language plpgsql;
