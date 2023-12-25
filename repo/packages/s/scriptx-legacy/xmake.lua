package("scriptx-legacy")
    on_load(function (package)
        package:set("installdir", os.scriptdir())
    end)

    on_fetch(function (package)
        local result = {}
        result.links = "scriptx-legacy"
        result.linkdirs = package:installdir("lib")
        result.includedirs = package:installdir("include")
        return result
    end)
