include($$PWD/additional.pri)

TEMPLATE = subdirs
SUBDIRS += ym ayfly dca v2m psf sc68 asap fc14

contains(CONFIG, ADPLUG_PLUGIN){
    SUBDIRS += adplug
}

contains(CONFIG, OPTIMFROG_PLUGIN){
    SUBDIRS += optimfrog
}
