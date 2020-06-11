##    Copyright 2015-2020 Christian Gunning (code@x14n.org)
##    This file is part of Skeeter Buster.

library(rgdal)
library(rgeos)

prep_hull <- function(
    spdf, block_col='block_number',
    ## exclude malformed blocks - tiny, huge
    ## typically singletons or same block id used for multiple
    max.blk.len = 1e3,
    min.blk.area = 200
){
    .blk <- spdf[[block_col]]
    ## for each block, subset, get hull as spPolDf
    .lhull = llply(unique(.blk), function(this_blk) {
        ## pull out polygons in *this* blk
        this = subset(spdf, .blk == this_blk)
        ## get wrap wrapper
        hull = gConvexHull(this, id=this_blk)
        ## remove blocks that are points, lines, or huge 
        ## e.g. errors
        if ( !is(hull, 'SpatialPolygons') | ( gLength(hull) > max.blk.len) | gArea(hull) < min.blk.area) {
            return()
        }
        .data = data.frame(row.names=this_blk, id=this_blk)
        ret = SpatialPolygonsDataFrame(hull, 
            data=.data,
            match.ID=FALSE
        )
        ret
    })
    ## remove missing, combine back into SPDF
    .lhull = Filter(function(x) !is.null(x), .lhull)
    ret = do.call(rbind, .lhull)
    ret
}

## take list of data.tables of distances
## used by prep_block, prep_point
prep_knn_helper <- function(
    ldist, kk, 
    max.dist, weight.pow, weight.digits
) {
    ## list to single data.table
    ret.dist = rbindlist(ldist)
    ## sort by dist
    setkey(ret.dist, self, dist)
    ## remove self
    ret.dist <- subset(ret.dist, self != link)
    ## get k nearest neighbor blocks
    ret.dist = ret.dist[,
        by=self, j=head(.SD,kk)
    ]
    ## should work for dist = 0
    ## pow = 0, equal weights
    ## pow = 1: weight by inverse distance, 
    ret.dist[, weight := round(max.dist/(1+dist)^weight.pow, weight.digits)]
    ret.dist
}

## take SpatialPointsDataFrame w/block col
## get knn network between blocks 
## coords should be in UTM
# all units in meters
## uses mclapply (optional?)
prep_block <- function(
    ## from prep_hull
    hull, 
    ## knn params: blocks (hull around points)
    kk=4,
    max.dist=100,
    weight.pow=0,
    ## round out false precision
    weight.digits = 3
) {
    ## buffer the hull of each block - fast
    .buff = gBuffer(hull, byid=T, 
        width=max.dist, quadsegs=3
    )
    ## for each buffer, 
    ## subset intersecting blocks, compute distance
    ## loop by id == rowname == block
    ldist = lapply(hull$id, function(the.blk) {
        ## current block
        the.hull = subset(hull, id == the.blk)
        ## get buffer for this block
        the.buff = subset(.buff, id==the.blk)
        ## blocks that touch / cross buffer 
        inbuff = subset(hull, 
            ## 1-col bool matrix of intersections
            as.vector(gIntersects(
                the.buff, hull, byid=T
            ))
        )
        ## distance matrix
        dist = gDistance(the.hull, inbuff, byid=T)
        ## as data.table
        ret = data.table(
            self = as.character(the.blk),
            link=rownames(dist),
            dist=round(as.vector(dist),3)
        )
        ret
    })
    ret.dist <- prep_knn_helper(
        ldist, kk, max.dist, 
        weight.pow, weight.digits
    )
    return(ret.dist)
}

## as for prep_block, but points are much easier
prep_point <- function(
    spdf, loc_col = 'location_code',
    ## knn params: locations (points)
    kk=6,
    max.dist=20,
    ## 0 = equal weights, 1=1/dist weights
    weight.pow=0,
    ## round out false precision
    weight.digits = 3
){
    ## prep
    row.names(spdf) <- spdf[[loc_col]]
    ## process by block
    blk <- unique(spdf$block_number)
    ldist <- lapply(blk, function(.blk) {
        blk.dat <- subset(spdf, block_number==.blk)
        ## for each point in block
        blk.dist <- gDistance(blk.dat, byid=T)
        blk.dist <- lapply(1:nrow(blk.dist), function(ii) {
            self = row.names(blk.dist)[ii]
            ## one row
            a_row <- blk.dist[ii,]
            ## from matrix to vector (different than df) 
            dist <- a_row[a_row <= max.dist]
            if(is.null(names(dist))) return()
            ## as data.table
            inret = data.table(
                self = self,
                link = names(dist),
                ## could be integer, but...
                dist=round(dist,3)
            )
            inret
        })
        ret <- rbindlist(blk.dist)
    })
    ret.dist <- prep_knn_helper(
        ldist, kk, max.dist, 
        weight.pow, weight.digits
    )
    ret.dist
}
