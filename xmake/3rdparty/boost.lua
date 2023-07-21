boost_includes_dir = "$(projectdir)/3rdparty/boost"
target("boost")
    set_group("boost")
    set_kind("headeronly")
    add_includedirs(boost_includes_dir, {public = true})