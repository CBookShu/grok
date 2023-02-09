local ltime = {}

--[[
local d = os.date()
local y = os.date("%Y")
local month = os.date("%m")
local d = os.date("%d")
local h = os.date("%H")
local minute = os.date("%M")
local s = os.date("%S")
]]

function ltime.getlocaltime()
    local lst = os.date("*t")
    local st = {
        year = lst.year,
        month = lst.month,
        day = lst.day,
        hour = lst.hour,
        minute = lst.min,
        second = lst.sec,
    }
    return st
end

function ltime.getoffsettime(diff)
    local t = os.time() + diff
    local lst = os.date("*t", t)
    local st = {
        year = lst.year,
        month = lst.month,
        day = lst.day,
        hour = lst.hour,
        minute = lst.min,
        second = lst.sec,
    }
    return st
end

function ltime.getdatenum(st)
    st = st or ltime.getlocaltime()
    local v = st.day
    v = v + st.month * 100
    v = v + st.year * 10000
    return v
end

function ltime.getmonthnum(st)
    st = st or ltime.getlocaltime()
    local v = st.month
    v = v + st.year * 100
    return v
end

function ltime.gettimenum(st)
    st = st or ltime.getlocaltime()
    local v = st.second
    v = v + st.minute * 100
    v = v + st.hour * 10000
    v = v + st.day * 1000000
    v = v + st.month * 100000000
    v = v + st.year * 10000000000
    return v
end

function ltime.gethournum(st)
    st = st or ltime.getlocaltime()
    local v = st.hour
    v = v + st.day * 100
    v = v + st.month * 10000
    v = v + st.year * 1000000
    return v
end

function ltime.getminutenum(st)
    st = st or ltime.getlocaltime()
    local v = st.minute
    v = v + st.hour * 100
    v = v + st.day * 10000
    v = v + st.month * 1000000
    v = v + st.year * 100000000
    return v
end

function ltime.parsetime(t)
    -- 拆分为年月日时分秒
    local function getleft(tm, n)
        local b = 10 ^ n
        local r = tm % b
        tm = math.floor(tm/b)
        return tm, r
    end
    local y1,M1,d1,h1,m1,s1
    t,s1 = getleft(t,2)
    t,m1 = getleft(t,2)
    t,h1 = getleft(t,2)
    t,d1 = getleft(t,2)
    t,M1 = getleft(t,2)
    t,y1 = getleft(t,4)
    return {year = y1,month = M1,day = d1,hour = h1,minute = m1,second = s1}
end

function ltime.getclocknum(st)
    st = st or ltime.getlocaltime()
    local v = st.second
    v = v + st.minute * 100
    v = v + st.hour * 10000
    return v
end

function ltime.getweekdatenum(st)
    st = st or ltime.getlocaltime()
    local week = ltime.getweekvalue(st.year,st.month,st.day)
    local datetag = ltime.getdatenum()
    return ltime.nextday(datetag, 1-week)
end

function ltime.getweekvalue(y,m,d)
    if m == 1 or m == 2 then
        m = m + 12
        y = y - 1
    end
    local m1,_ = math.modf(3 * (m + 1) / 5)
    local m2,_ = math.modf(y / 4)
    local m3,_ = math.modf(y / 100)
    local m4,_ = math.modf(y / 400)

    local iWeek = (d + 2 * m + m1 + y + m2 - m3  + m4 ) % 7
    local weekTab = {
        ["0"] = 1,
        ["1"] = 2,
        ["2"] = 3,
        ["3"] = 4,
        ["4"] = 5,
        ["5"] = 6,
        ["6"] = 7,
    }
    return weekTab[tostring(iWeek)]
end

function ltime.nextmonth(tag,n)
    local y = math.floor( tag / 100  )
    local m = math.floor( tag % 100 )
    m = m + n
    if m > 0 then
        local dy = (m - 1) / 12
        y = math.floor( y + dy ) 
        m =  math.floor((m - 1) % 12 + 1)
    else
        local base =  math.floor(math.abs(m) / 12 + 1)
        m = m + base * 12
        y = y - base
    end

    local v = m
    v = v + y * 100
    return v
end

function ltime.month2_month1(m2, m1)
    local y2 = math.floor( m2 / 100  )
    local m2 = math.floor( m2 % 100 )

    local y1 = math.floor( m1 / 100  )
    local m1 = math.floor( m1 % 100 )

    local dy = y2 - y1
    local dm = m2 - m1

    local v = dy * 12 + dm
    return v
end

function ltime.nextday(day, n)
    local y = math.floor( day / 10000  )
    local m = math.floor( (day / 100) % 100 )
    local d = math.floor( day % 100 )
    local t = os.time({year = y, month = m, day = d})
    t = t + n * 24 * 60 * 60
    local st = os.date("!*t", t)
    local v = st.day
    v = v + st.month * 100
    v = v + st.year * 10000
    return v
end

-- 将20221104这种格式转为时间戳
function ltime.gettimestampbydaynum(daynum)
    local y = math.floor( daynum / 10000  )
    local m = math.floor( (daynum / 100) % 100 )
    local d = math.floor( daynum % 100 )
    local t = os.time({year = y, month = m, day = d})
    return t
end

-- 将20221104123050这种格式转为时间戳
function ltime.gettimestampbytimenum(timenum)
    local timestruct = ltime.parsetime(timenum)
    local t = os.time({ year = timestruct.year, month = timestruct.month, day = timestruct.day, 
                    hour = timestruct.hour, min = timestruct.minute, sec = timestruct.second })
    return t
end

-- 针对20221104这种格式，计算t1 到 t2有多少秒
function ltime.day2_day1(day2, day1)
    local y1 = math.floor( day1 / 10000  )
    local m1 = math.floor( (day1 / 100) % 100 )
    local d1 = math.floor( day1 % 100 )
    local t1 = os.time({year = y1, month = m1, day = d1})

    local y2 = math.floor( day2 / 10000  )
    local m2 = math.floor( (day2 / 100) % 100 )
    local d2 = math.floor( day2 % 100 )
    local t2 = os.time({year = y2, month = m2, day = d2})
    return math.floor((t2 - t1)/(24 * 60 * 60)) 
end

-- 针对20221104123050这种格式，计算t1 到 t2有多少秒
function ltime.time2_time1(t2, t1)
    local function getleft(t, n)
        local b = 10 ^ n
        local r = t % b
        t = math.floor(t/b)
        return t, r
    end
    local y1,M1,d1,h1,m1,s1
    t1,s1 = getleft(t1,2)
    t1,m1 = getleft(t1,2)
    t1,h1 = getleft(t1,2)
    t1,d1 = getleft(t1,2)
    t1,M1 = getleft(t1,2)
    t1,y1 = getleft(t1,4)
    local ut1 = os.time({year = y1, month = M1, day = d1, hour=h1,min=m1,sec=s1})
    
    local y2,M2,d2,h2,m2,s2
    t2,s2 = getleft(t2,2)
    t2,m2 = getleft(t2,2)
    t2,h2 = getleft(t2,2)
    t2,d2 = getleft(t2,2)
    t2,M2 = getleft(t2,2)
    t2,y2 = getleft(t2,4)
    local ut2 = os.time({year = y2, month = M2, day = d2, hour=h2,min=m2,sec=s2})
    return ut2 - ut1
end

-- 针对20221104123050这种格式，增加t秒后新的时间
function ltime.timenum_add(t1, t)
    local timestamp = ltime.gettimestampbytimenum(t1)
    local lst = os.date("*t", timestamp + t)
    local st = {
        year = lst.year,
        month = lst.month,
        day = lst.day,
        hour = lst.hour,
        minute = lst.min,
        second = lst.sec,
    }
    local res = ltime.gettimenum(st)
    return res
end

return ltime
