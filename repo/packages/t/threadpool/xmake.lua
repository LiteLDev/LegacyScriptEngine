package("threadpool")
    on_load(function (package)
        package:set("installdir", os.scriptdir())
    end)

    on_fetch(function (package)
        local result = {}
        result.links = "threadpool"
        result.includedirs = package:installdir("include")
        return result
    end)
