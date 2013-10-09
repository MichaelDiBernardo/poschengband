/* File: wild.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke,
 * Robert Ruehlmann
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

/* Purpose: Wilderness generation 
 *
 * CTK: Added infinitely scrolling wilderness support (hackish).
 */

#include "angband.h"

s16b *_cave[MAX_HGT];

static void _apply_glow(bool all)
{
    int y,x;
    cave_type *c_ptr;
    feature_type *f_ptr;

    for (y = 0; y < cur_hgt; y++)
    {
        for (x = 0; x < cur_wid; x++)
        {
            /* Get the cave grid */
            c_ptr = &cave[y][x];
            if (all || (c_ptr->info & CAVE_TEMP))
            {
                if (is_daytime())
                {
                    /* Assume lit */
                    c_ptr->info |= (CAVE_GLOW);

                    /* Hack -- Memorize lit grids if allowed */
                    if (view_perma_grids) c_ptr->info |= (CAVE_MARK);
                }
                else
                {
                    /* Feature code (applying "mimic" field) */
                    f_ptr = &f_info[get_feat_mimic(c_ptr)];

                    if (!is_mirror_grid(c_ptr) && !have_flag(f_ptr->flags, FF_QUEST_ENTER) &&
                        !have_flag(f_ptr->flags, FF_ENTRANCE))
                    {
                        /* Assume dark */
                        c_ptr->info &= ~(CAVE_GLOW);

                        /* Darken "boring" features */
                        if (!have_flag(f_ptr->flags, FF_REMEMBER))
                        {
                            /* Forget the grid */
                            c_ptr->info &= ~(CAVE_MARK);
                        }
                    }
                    else if (have_flag(f_ptr->flags, FF_ENTRANCE))
                    {
                        /* Assume lit */
                        c_ptr->info |= (CAVE_GLOW);

                        /* Hack -- Memorize lit grids if allowed */
                        if (view_perma_grids) c_ptr->info |= (CAVE_MARK);
                    }
                }
                if (!all)
                    c_ptr->info &= ~CAVE_TEMP;
            }
        }
    }
}

static void _scroll_grid(int src_y, int src_x, int dest_y, int dest_x)
{
    cave_type *src = &cave[src_y][src_x];
    if (in_bounds(dest_y, dest_x))
    {
        cave_type *dest = &cave[dest_y][dest_x];
        if (dest->m_idx)
            delete_monster_idx(dest->m_idx);
        if (dest->o_idx)
            delete_object_idx(dest->o_idx);

        *dest = *src;
        WIPE(src, cave_type);
        src->info |= CAVE_TEMP;  /* Mark for _apply_glow */

        if (dest->m_idx)
        {
            m_list[dest->m_idx].fy = dest_y;
            m_list[dest->m_idx].fx = dest_x;
        }
        if (dest->o_idx)
        {
            o_list[dest->o_idx].iy = dest_y;
            o_list[dest->o_idx].ix = dest_x;
        }
    }
    else
    {
        if (src->m_idx)
            delete_monster_idx(src->m_idx);
        if (src->o_idx)
            delete_object_idx(src->o_idx);
        WIPE(src, cave_type);
        src->info |= CAVE_TEMP;  /* Mark for _apply_glow */
    }
}

bool _scroll_panel(int dy, int dx)
{
    int y, x;
    int wid, hgt;

    get_screen_size(&wid, &hgt);

    y = panel_row_min + dy;
    x = panel_col_min + dx;

    if (y > cur_hgt - hgt) y = cur_hgt - hgt;
    if (y < 0) y = 0;

    if (x > cur_wid - wid) x = cur_wid - wid;
    if (x < 0) x = 0;

    if (y != panel_row_min || x != panel_col_min)
    {
        panel_row_min = y;
        panel_col_min = x;

        panel_bounds_center();
    /*  panel_lock = TRUE; */

        p_ptr->update |= PU_MONSTERS;
        p_ptr->redraw |= PR_MAP;
        handle_stuff();
        return TRUE;
    }
    return FALSE;
}

static void _scroll_cave(int dy, int dx)
{
    int y, x;

    if (dy == 0 && dx == 0)
        return;

    if (dy <= 0 && dx <= 0)
    {
        for (y = 1; y < MAX_HGT - 1; y++)
        {
            for (x = 1; x < MAX_WID - 1; x++)
                _scroll_grid(y, x, y + dy, x + dx);
        }
    }
    else if (dy >= 0 && dx >= 0)
    {
        for (y = MAX_HGT - 2; y > 0 ; y--)
        {
            for (x = MAX_WID - 2; x > 0; x--)
                _scroll_grid(y, x, y + dy, x + dx);
        }
    }
    else if (dy > 0 && dx < 0)
    {
        for (y = MAX_HGT - 2; y > 0 ; y--)
        {
            for (x = 1; x < MAX_WID - 1; x++)
                _scroll_grid(y, x, y + dy, x + dx);
        }
    }
    else if (dy < 0 && dx > 0)
    {
        for (y = 1; y < MAX_HGT - 1; y++)
        {
            for (x = MAX_WID - 2; x > 0; x--)
                _scroll_grid(y, x, y + dy, x + dx);
        }
    }
    else
    {
        /* ooops! */
    }

    py += dy;
    px += dx;

/*  verify_panel_aux(PANEL_FORCE_CENTER); */
    _scroll_panel(dy, dx);

    p_ptr->update |= PU_DISTANCE;
    p_ptr->redraw |= PR_MAP;
    p_ptr->window |= PW_OVERHEAD | PW_DUNGEON;
}

static void generate_area(int y, int x, int dy, int dx);

void wilderness_move_player(int old_y, int old_x)
{
    int old_qy = (old_y-1) / WILD_SCROLL_CY;
    int old_qx = (old_x-1) / WILD_SCROLL_CX;
    int qy = (py-1) / WILD_SCROLL_CY;
    int qx = (px-1) / WILD_SCROLL_CX;
    int wild_qx = qx + p_ptr->wilderness_dx;
    int wild_qy = qy + p_ptr->wilderness_dy;
    int dy = qy - old_qy;
    int dx = qx - old_qx;
    int x,y;

    /* Hack: Player was placed on the edge of a wilderness tile. Force a scroll! 
       TODO: Prevent this from happening in the first place! */
    if (!dy && !dx)
    {
        if (qy == 3)
            dy = 1;
        if (qy == 0)
            dy = -1;
        if (qx == 3)
            dx = 1;
        if (qx == 0)
            dx = -1;
    }

#ifdef _DEBUG
    c_put_str(TERM_WHITE, format("P:%3d/%3d", px, py), 26, 0);
    c_put_str(TERM_WHITE, format("W:%3d/%3d", p_ptr->wilderness_x, p_ptr->wilderness_y), 27, 0);
    c_put_str(TERM_WHITE, format("D:%3d/%3d", p_ptr->wilderness_dx, p_ptr->wilderness_dy), 28, 0);
    c_put_str(TERM_WHITE, format("O:%3d/%3d", old_qx, old_qy), 29, 0);
    c_put_str(TERM_WHITE, format("Q:%3d/%3d", qx, qy), 30, 0);
    c_put_str(TERM_WHITE, format("S:%3d/%3d", dx, dy), 31, 0);
#endif

    if (!dy && !dx)
        return;

    /* TODO: Check to make sure scrolling is allowed (e.g. at wilderness edge) */

    _scroll_cave(-dy*WILD_SCROLL_CY, -dx*WILD_SCROLL_CX);
    
    p_ptr->wilderness_dy += dy;
    p_ptr->wilderness_dx += dx;

    /* Patch up player's wilderness coordinates. If they are standing on (X,Y)
       then make sure wilderness_x and y reflect this. */
    if (wild_qx >= 4)
    {
        p_ptr->wilderness_x++;
        p_ptr->wilderness_dx -= 4;
    }
    else if (wild_qx < 0)
    {
        p_ptr->wilderness_x--;
        p_ptr->wilderness_dx += 4;
    }
    if (wild_qy >= 4)
    {
        p_ptr->wilderness_y++;
        p_ptr->wilderness_dy -= 4;
    }
    else if (wild_qy < 0)
    {
        p_ptr->wilderness_y--;
        p_ptr->wilderness_dy += 4;
    }

    /* TODO: This is the lazy way to scroll in new wilderness info! */
    for (x = -1; x <= 1; x++)
    {
        for (y = -1; y <= 1; y++)
        {
            generate_area(
                p_ptr->wilderness_y + y, 
                p_ptr->wilderness_x + x, 
                (y*4 - p_ptr->wilderness_dy) * WILD_SCROLL_CY, 
                (x*4 - p_ptr->wilderness_dx) * WILD_SCROLL_CX
            );
        }
    }
    _apply_glow(FALSE);

#ifdef _DEBUG
    c_put_str(TERM_WHITE, format("P:%3d/%3d", px, py), 26, 0);
    c_put_str(TERM_WHITE, format("W:%3d/%3d", p_ptr->wilderness_x, p_ptr->wilderness_y), 27, 0);
    c_put_str(TERM_WHITE, format("D:%3d/%3d", p_ptr->wilderness_dx, p_ptr->wilderness_dy), 28, 0);
#endif
}

static void set_floor_and_wall_aux(s16b feat_type[100], feat_prob prob[DUNGEON_FEAT_PROB_NUM])
{
    int lim[DUNGEON_FEAT_PROB_NUM], cur = 0, i;

    lim[0] = prob[0].percent;
    for (i = 1; i < DUNGEON_FEAT_PROB_NUM; i++) lim[i] = lim[i - 1] + prob[i].percent;

    /* Paranoia */
    if (lim[DUNGEON_FEAT_PROB_NUM - 1] < 100) lim[DUNGEON_FEAT_PROB_NUM - 1] = 100;

    for (i = 0; i < 100; i++)
    {
        while (i == lim[cur]) cur++;
        feat_type[i] = prob[cur].feat;
    }
}

/*
 * Fill the arrays of floors and walls in the good proportions
 */
void set_floor_and_wall(byte type)
{
    static byte cur_type = 255;
    dungeon_info_type *d_ptr;

    /* Already filled */
    if (cur_type == type) return;

    cur_type = type;
    d_ptr = &d_info[type];

    set_floor_and_wall_aux(floor_type, d_ptr->floor);
    set_floor_and_wall_aux(fill_type, d_ptr->fill);

    feat_wall_outer = d_ptr->outer_wall;
    feat_wall_inner = d_ptr->inner_wall;
    feat_wall_solid = d_ptr->outer_wall;
}


/*
 * Helper for plasma generation.
 */
static void perturb_point_mid(int x1, int x2, int x3, int x4,
              int xmid, int ymid, int rough, int depth_max)
{
    /*
     * Average the four corners & perturb it a bit.
     * tmp is a random int +/- rough
     */
    int tmp2 = rough*2 + 1;
    int tmp = randint1(tmp2) - (rough + 1);

    int avg = ((x1 + x2 + x3 + x4) / 4) + tmp;

    /* Division always rounds down, so we round up again */
    if (((x1 + x2 + x3 + x4) % 4) > 1)
        avg++;

    /* Normalize */
    if (avg < 0) avg = 0;
    if (avg > depth_max) avg = depth_max;

    /* Set the new value. */
    _cave[ymid][xmid] = avg;
}


static void perturb_point_end(int x1, int x2, int x3,
              int xmid, int ymid, int rough, int depth_max)
{
    /*
     * Average the three corners & perturb it a bit.
     * tmp is a random int +/- rough
     */
    int tmp2 = rough * 2 + 1;
    int tmp = randint0(tmp2) - rough;

    int avg = ((x1 + x2 + x3) / 3) + tmp;

    /* Division always rounds down, so we round up again */
    if ((x1 + x2 + x3) % 3) avg++;

    /* Normalize */
    if (avg < 0) avg = 0;
    if (avg > depth_max) avg = depth_max;

    /* Set the new value. */
    _cave[ymid][xmid] = avg;
}


/*
 * A generic function to generate the plasma fractal.
 * Note that it uses ``cave_feat'' as temporary storage.
 * The values in ``cave_feat'' after this function
 * are NOT actual features; They are raw heights which
 * need to be converted to features.
 */
static void plasma_recursive(int x1, int y1, int x2, int y2,
                 int depth_max, int rough)
{
    /* Find middle */
    int xmid = (x2 - x1) / 2 + x1;
    int ymid = (y2 - y1) / 2 + y1;

    /* Are we done? */
    if (x1 + 1 == x2) return;

    perturb_point_mid(_cave[y1][x1], _cave[y2][x1], _cave[y1][x2],
        _cave[y2][x2], xmid, ymid, rough, depth_max);

    perturb_point_end(_cave[y1][x1], _cave[y1][x2], _cave[ymid][xmid],
        xmid, y1, rough, depth_max);

    perturb_point_end(_cave[y1][x2], _cave[y2][x2], _cave[ymid][xmid],
        x2, ymid, rough, depth_max);

    perturb_point_end(_cave[y2][x2], _cave[y2][x1], _cave[ymid][xmid],
        xmid, y2, rough, depth_max);

    perturb_point_end(_cave[y2][x1], _cave[y1][x1], _cave[ymid][xmid],
        x1, ymid, rough, depth_max);


    /* Recurse the four quadrants */
    plasma_recursive(x1, y1, xmid, ymid, depth_max, rough);
    plasma_recursive(xmid, y1, x2, ymid, depth_max, rough);
    plasma_recursive(x1, ymid, xmid, y2, depth_max, rough);
    plasma_recursive(xmid, ymid, x2, y2, depth_max, rough);
}


#define MAX_FEAT_IN_TERRAIN 18

/*
 * The default table in terrain level generation.
 */
static s16b terrain_table[MAX_WILDERNESS][MAX_FEAT_IN_TERRAIN];


static void generate_wilderness_area(int terrain, u32b seed)
{
    int x1, y1;
    int table_size = sizeof(terrain_table[0]) / sizeof(s16b);
    int roughness = 1; /* The roughness of the level. */

    /* The outer wall is easy */
    if (terrain == TERRAIN_EDGE)
    {
        /* Create level background */
        for (y1 = 0; y1 < MAX_HGT; y1++)
        {
            for (x1 = 0; x1 < MAX_WID; x1++)
            {
                _cave[y1][x1] = feat_permanent;
            }
        }

        /* We are done already */
        return;
    }


    /* Hack -- Use the "simple" RNG */
    Rand_quick = TRUE;

    /* Hack -- Induce consistant town layout */
    Rand_value = seed;

    /* Create level background */
    for (y1 = 0; y1 < MAX_HGT; y1++)
    {
        for (x1 = 0; x1 < MAX_WID; x1++)
        {
            _cave[y1][x1] = table_size / 2;
        }
    }

    /*
     * Initialize the four corners
     * ToDo: calculate the medium height of the adjacent
     * terrains for every corner.
     */
    _cave[1][1] = randint0(table_size);
    _cave[MAX_HGT-2][1] = randint0(table_size);
    _cave[1][MAX_WID-2] = randint0(table_size);
    _cave[MAX_HGT-2][MAX_WID-2] = randint0(table_size);

    /* Hack -- preserve four corners */
    {
        s16b north_west = _cave[1][1];
        s16b south_west = _cave[MAX_HGT - 2][1];
        s16b north_east = _cave[1][MAX_WID - 2];
        s16b south_east = _cave[MAX_HGT - 2][MAX_WID - 2];

        /* x1, y1, x2, y2, num_depths, roughness */
        plasma_recursive(1, 1, MAX_WID-2, MAX_HGT-2, table_size-1, roughness);

        /* Hack -- copyback four corners */
        _cave[1][1] = north_west;
        _cave[MAX_HGT - 2][1] = south_west;
        _cave[1][MAX_WID - 2] = north_east;
        _cave[MAX_HGT - 2][MAX_WID - 2] = south_east;

        for (y1 = 1; y1 < MAX_HGT - 1; y1++)
        {
            for (x1 = 1; x1 < MAX_WID - 1; x1++)
            {
                _cave[y1][x1] = terrain_table[terrain][_cave[y1][x1]];
            }
        }
    }
    /* Use the complex RNG */
    Rand_quick = FALSE;
}



/*
 * Load a town or generate a terrain level using "plasma" fractals.
 *
 * x and y are the coordinates of the area in the wilderness.
 * Border and corner are optimization flags to speed up the
 * generation of the fractal terrain.
 * If border is set then only the border of the terrain should
 * be generated (for initializing the border structure).
 * If corner is set then only the corners of the area are needed.
 */
static void generate_area(int y, int x, int dy, int dx)
{
    int x1, y1;

    if (abs(dy) >= MAX_HGT - 2) return;
    if (abs(dx) >= MAX_WID - 2) return;

    {
        int terrain = wilderness[y][x].terrain;
        u32b seed = wilderness[y][x].seed;

        generate_wilderness_area(terrain, seed);

        if (wilderness[y][x].road && !wilderness[y][x].town)
        {
            _cave[MAX_HGT/2][MAX_WID/2] = feat_floor;

            if (wilderness[y-1][x].road)
            {
                /* North road */
                for (y1 = 1; y1 < MAX_HGT/2; y1++)
                {
                    x1 = MAX_WID/2;
                    _cave[y1][x1] = feat_floor;
                }
            }

            if (wilderness[y+1][x].road)
            {
                /* South road */
                for (y1 = MAX_HGT/2; y1 < MAX_HGT - 1; y1++)
                {
                    x1 = MAX_WID/2;
                    _cave[y1][x1] = feat_floor;
                }
            }

            if (wilderness[y][x+1].road)
            {
                /* East road */
                for (x1 = MAX_WID/2; x1 < MAX_WID - 1; x1++)
                {
                    y1 = MAX_HGT/2;
                    _cave[y1][x1] = feat_floor;
                }
            }

            if (wilderness[y][x-1].road)
            {
                /* West road */
                for (x1 = 1; x1 < MAX_WID/2; x1++)
                {
                    y1 = MAX_HGT/2;
                    _cave[y1][x1] = feat_floor;
                }
            }
        }

        /* Copy features from scratch buffer to true cave data, applying a delta for scrolling */
        for (y1 = 1; y1 < MAX_HGT - 1; y1++)
        {
            for (x1 = 1; x1 < MAX_WID - 1; x1++)
            {
                int y2 = y1 + dy;
                int x2 = x1 + dx;

                if (!in_bounds(y2, x2)) continue;
                cave[y2][x2].feat = _cave[y1][x1];
            }
        }

        /* Create the town on top of default terrain */
        if (wilderness[y][x].town)
        {
            /* Hack: We assume only 1 town can be on the viewport at a time ... */
            /* Pref files *require* p_ptr->town_num to be set!! */
            p_ptr->town_num = wilderness[y][x].town;

            /* Reset the buildings */
            init_buildings();

            /* Initialize the town */
            init_flags = INIT_CREATE_DUNGEON;
            init_dx = dx;
            init_dy = dy;
            process_dungeon_file("t_info.txt", 0, 0, MAX_HGT, MAX_WID);
            init_flags = 0;
            init_dx = 0;
            init_dy = 0;

            p_ptr->visit |= (1L << (p_ptr->town_num - 1));
        }


        /* Ah ... well, our _cave scratch buffer can't handle the stairs. */
        if (wilderness[y][x].entrance && !wilderness[y][x].town && (p_ptr->total_winner || !(d_info[wilderness[y][x].entrance].flags1 & DF1_WINNER)))
        {
            int y2, x2;
            int which = wilderness[y][x].entrance;

            /* Hack -- Use the "simple" RNG */
            Rand_quick = TRUE;

            /* Hack -- Induce consistant town layout */
            Rand_value = wilderness[y][x].seed;

            y2 = rand_range(6, cur_hgt - 6) + dy;
            x2 = rand_range(6, cur_wid - 6) + dx;

            if (in_bounds(y2, x2))
            {
                if (dungeon_flags[which] & DUNGEON_NO_ENTRANCE)
                {
                    cave[dy][dx].feat = feat_mountain;
                }
                else
                {
                    cave[y2][x2].feat = feat_entrance;
                    cave[y2][x2].special = which;
                }
            }
            /* Use the complex RNG */
            Rand_quick = FALSE;
        }

    }
}

/*
 * Build the wilderness area outside of the town.
 */
void wilderness_gen(void)
{
    int i, y, x, lim;
    cave_type *c_ptr;
    feature_type *f_ptr;

    /* Big town */
    cur_hgt = MAX_HGT;
    cur_wid = MAX_WID;

    /* Assume illegal panel */
    panel_row_min = cur_hgt;
    panel_col_min = cur_wid;

    /* Init the wilderness */

    process_dungeon_file("w_info.txt", 0, 0, max_wild_y, max_wild_x);

    /* Prepare allocation table */
    get_mon_num_prep(get_monster_hook(), NULL);

    /* TODO: There might be multiple squares visible at the same time due to scrolling ... */
    p_ptr->town_num = wilderness[p_ptr->wilderness_y][p_ptr->wilderness_x].town;
    base_level = wilderness[p_ptr->wilderness_y][p_ptr->wilderness_x].level;
    dun_level = 0;
    monster_level = base_level;
    object_level = base_level;

    /* The current cave[][] is a 4x4 viewport on a very large wilderness map.
       Picture a "cursor" which you can slide about on the map and peer into 
       the wilderness. Coordinates (wilderness_x and y) and offsets (wilderness_dx and dy) 
       apply to this cursor. */

/*
p_ptr->wilderness_dy = 1;
p_ptr->wilderness_dx = 1;
*/
    for (x = -1; x <= 1; x++)
    {
        for (y = -1; y <= 1; y++)
        {
            generate_area(
                p_ptr->wilderness_y + y, 
                p_ptr->wilderness_x + x, 
                (y*4 - p_ptr->wilderness_dy) * WILD_SCROLL_CY, 
                (x*4 - p_ptr->wilderness_dx) * WILD_SCROLL_CX
            );
        }
    }

    /* Special boundary walls -- North */
    for (i = 0; i < MAX_WID; i++)
    {
        cave[0][i].feat = feat_permanent;
        cave[0][i].mimic = feat_dark_pit;
    }

    /* Special boundary walls -- South */
    for (i = 0; i < MAX_WID; i++)
    {
        cave[MAX_HGT - 1][i].feat = feat_permanent;
        cave[MAX_HGT - 1][i].mimic = feat_dark_pit;
    }

    /* Special boundary walls -- West */
    for (i = 0; i < MAX_HGT; i++)
    {
        cave[i][0].feat = feat_permanent;
        cave[i][0].mimic = feat_dark_pit;
    }

    /* Special boundary walls -- East */
    for (i = 0; i < MAX_HGT; i++)
    {
        cave[i][MAX_WID - 1].feat = feat_permanent;
        cave[i][MAX_WID - 1].mimic = feat_dark_pit;
    }

    /* Light up or darken the area */
    _apply_glow(TRUE);

    /* When teleporting from town to town, look for the building that offers the
       teleport service to place the player */
    if (p_ptr->teleport_town)
    {
        for (y = 0; y < cur_hgt; y++)
        {
            for (x = 0; x < cur_wid; x++)
            {
                c_ptr = &cave[y][x];

                f_ptr = &f_info[c_ptr->feat];
                if (have_flag(f_ptr->flags, FF_BLDG))
                {
                    if ((f_ptr->subtype == 4) || ((p_ptr->town_num == 1) && (f_ptr->subtype == 0)))
                    {
                        if (c_ptr->m_idx) delete_monster_idx(c_ptr->m_idx);
                        p_ptr->oldpy = y;
                        p_ptr->oldpx = x;
                    }
                }
            }
        }
        p_ptr->teleport_town = FALSE;
    }
    /* When leaving the dungeon, look for the wilderness stairs to place the player */
    else if (p_ptr->leaving_dungeon)
    {
        for (y = 0; y < cur_hgt; y++)
        {
            for (x = 0; x < cur_wid; x++)
            {
                /* Get the cave grid */
                c_ptr = &cave[y][x];

                if (cave_have_flag_grid(c_ptr, FF_ENTRANCE))
                {
                    if (c_ptr->m_idx) delete_monster_idx(c_ptr->m_idx);
                    p_ptr->oldpy = y;
                    p_ptr->oldpx = x;
                }
            }
        }
        p_ptr->teleport_town = FALSE;
    }

    player_place(p_ptr->oldpy, p_ptr->oldpx);
    /* p_ptr->leaving_dungeon = FALSE;*/

    lim = (generate_encounter==TRUE)?40:MIN_M_ALLOC_TN;

lim = 0;

    /* Make some residents */
    for (i = 0; i < lim; i++)
    {
        u32b mode = 0;

        if (!(generate_encounter || (one_in_(2) && (!p_ptr->town_num))))
            mode |= PM_ALLOW_SLEEP;

        /* Make a resident */
        (void)alloc_monster(generate_encounter ? 0 : 3, mode);
    }

    if(generate_encounter) ambush_flag = TRUE;
    generate_encounter = FALSE;

    /* Fill the arrays of floors and walls in the good proportions */
    set_floor_and_wall(0);

    /* Set rewarded quests to finished */
    for (i = 0; i < max_quests; i++)
    {
        if (quest[i].status == QUEST_STATUS_REWARDED)
            quest[i].status = QUEST_STATUS_FINISHED;
    }
}


static s16b conv_terrain2feat[MAX_WILDERNESS];

/*
 * Build the wilderness area.
 * -DG-
 */
void wilderness_gen_small()
{
    int i, j;

    /* To prevent stupid things */
    for (i = 0; i < MAX_WID; i++)
    for (j = 0; j < MAX_HGT; j++)
    {
        cave[j][i].feat = feat_permanent;
    }

    /* Init the wilderness */
    process_dungeon_file("w_info.txt", 0, 0, max_wild_y, max_wild_x);

    /* Fill the map */
    for (i = 0; i < max_wild_x; i++)
    for (j = 0; j < max_wild_y; j++)
    {
        if (wilderness[j][i].town && (wilderness[j][i].town != NO_TOWN))
        {
            cave[j][i].feat = feat_town;
            cave[j][i].special = wilderness[j][i].town;
        }
        else if (wilderness[j][i].road) cave[j][i].feat = feat_floor;
        else if (wilderness[j][i].entrance && (p_ptr->total_winner || !(d_info[wilderness[j][i].entrance].flags1 & DF1_WINNER)))
        {
            cave[j][i].feat = feat_entrance;
            cave[j][i].special = (byte)wilderness[j][i].entrance;
        }
        else cave[j][i].feat = conv_terrain2feat[wilderness[j][i].terrain];

        cave[j][i].info |= (CAVE_GLOW | CAVE_MARK);
    }

    cur_hgt = (s16b) max_wild_y;
    cur_wid = (s16b) max_wild_x;

    if (cur_hgt > MAX_HGT) cur_hgt = MAX_HGT;
    if (cur_wid > MAX_WID) cur_wid = MAX_WID;

    /* Assume illegal panel */
    panel_row_min = cur_hgt;
    panel_col_min = cur_wid;

    /* Place the player */
    px = p_ptr->wilderness_x;
    py = p_ptr->wilderness_y;

    p_ptr->town_num = 0;
}


typedef struct wilderness_grid wilderness_grid;

struct wilderness_grid
{
    int        terrain;    /* Terrain type */
    int        town;       /* Town number */
    s16b    level;        /* Level of the wilderness */
    byte    road;       /* Road */
    char    name[32];    /* Name of the town/wilderness */
};


static wilderness_grid w_letter[255];


/*
 * Parse a sub-file of the "extra info"
 */
errr parse_line_wilderness(char *buf, int ymin, int xmin, int ymax, int xmax, int *y, int *x)
{
    int i, num;
    char *zz[33];

    /* Unused */
    (void)ymin;
    (void)ymax;

    /* Paranoia */
    if (!(buf[0] == 'W')) return (PARSE_ERROR_GENERIC);

    switch (buf[2])
    {
        /* Process "W:F:<letter>:<terrain>:<town>:<road>:<name> */
    case 'J':
        return 0;
    case 'F':
    case 'E':
    {
        if ((num = tokenize(buf+4, 6, zz, 0)) > 1)
        {
            int index = zz[0][0];
            
            if (num > 1)
                w_letter[index].terrain = atoi(zz[1]);
            else
                w_letter[index].terrain = 0;
            
            if (num > 2)
                w_letter[index].level = atoi(zz[2]);
            else
                w_letter[index].level = 0;
            
            if (num > 3)
                w_letter[index].town = atoi(zz[3]);
            else
                w_letter[index].town = 0;
            
            if (num > 4)
                w_letter[index].road = atoi(zz[4]);
            else
                w_letter[index].road = 0;
            
            if (num > 5)
                strcpy(w_letter[index].name, zz[5]);
            else
                w_letter[index].name[0] = 0;
        }
        else
        {
                /* Failure */
            return (PARSE_ERROR_TOO_FEW_ARGUMENTS);
        }
        
        break;
    }
    
    /* Process "W:D:<layout> */
    /* Layout of the wilderness */
    case 'D':
    {
        /* Acquire the text */
        char *s = buf+4;
        
        /* Length of the text */
        int len = strlen(s);
        
        for (*x = xmin, i = 0; ((*x < xmax) && (i < len)); (*x)++, s++, i++)
        {
            int idx = s[0];
            
            wilderness[*y][*x].terrain = w_letter[idx].terrain;
            
            wilderness[*y][*x].level = w_letter[idx].level;
            
            wilderness[*y][*x].town = w_letter[idx].town;
            
            wilderness[*y][*x].road = w_letter[idx].road;
            
            strcpy(town[w_letter[idx].town].name, w_letter[idx].name);
        }
        
        (*y)++;
        
        break;
    }
    
    /* Process "W:P:<x>:<y> - starting position in the wilderness */
    case 'P':
    {
        if ((p_ptr->wilderness_x == 0) &&
            (p_ptr->wilderness_y == 0))
        {
            if (tokenize(buf+4, 2, zz, 0) == 2)
            {
                p_ptr->wilderness_y = atoi(zz[0]);
                p_ptr->wilderness_x = atoi(zz[1]);
                
                if ((p_ptr->wilderness_x < 1) ||
                    (p_ptr->wilderness_x > max_wild_x) ||
                    (p_ptr->wilderness_y < 1) ||
                    (p_ptr->wilderness_y > max_wild_y))
                {
                    return (PARSE_ERROR_OUT_OF_BOUNDS);
                }
            }
            else
            {
                return (PARSE_ERROR_TOO_FEW_ARGUMENTS);
            }
        }
        
        break;
    }
    
    default:
        /* Failure */
        return (PARSE_ERROR_UNDEFINED_DIRECTIVE);
    }
    
    for (i = 1; i < max_d_idx; i++)
    {
        if (!d_info[i].maxdepth) continue;
        wilderness[d_info[i].dy][d_info[i].dx].entrance = i;
        if (!wilderness[d_info[i].dy][d_info[i].dx].town)
            wilderness[d_info[i].dy][d_info[i].dx].level = d_info[i].mindepth;
    }

    /* Success */
    return (0);
}


/*
 * Generate the random seeds for the wilderness
 */
void seed_wilderness(void)
{
    int x, y;

    /* Init wilderness seeds */
    for (x = 0; x < max_wild_x; x++)
    {
        for (y = 0; y < max_wild_y; y++)
        {
            wilderness[y][x].seed = randint0(0x10000000);
            wilderness[y][x].entrance = 0;
        }
    }
}


/*
 * Pointer to wilderness_type
 */
typedef wilderness_type *wilderness_type_ptr;

/*
 * Initialize wilderness array
 */
errr init_wilderness(void)
{
    int i;

    /* Allocate the wilderness (two-dimension array) */
    C_MAKE(wilderness, max_wild_y, wilderness_type_ptr);
    C_MAKE(wilderness[0], max_wild_x * max_wild_y, wilderness_type);

    /* Init the other pointers */
    for (i = 1; i < max_wild_y; i++)
        wilderness[i] = wilderness[0] + i * max_wild_x;

    for (i = 0; i < MAX_HGT; i++)
        C_MAKE(_cave[i], MAX_WID, s16b);

    generate_encounter = FALSE;

    return 0;
}


static void init_terrain_table(int terrain, s16b feat_global, cptr fmt, ...)
{
    va_list vp;
    cptr    p;
    int     cur = 0;
    char    check = 'a';
    s16b    feat;
    int     num;

    /* Begin the varargs stuff */
    va_start(vp, fmt);

    /* Wilderness terrains on global map */
    conv_terrain2feat[terrain] = feat_global;

    /* Wilderness terrains on local map */
    for (p = fmt; *p; p++)
    {
        if (*p == check)
        {
            int lim;

            feat = (s16b)va_arg(vp, int);
            num = va_arg(vp, int);
            lim = cur + num;

            for (; (cur < lim) && (cur < MAX_FEAT_IN_TERRAIN); cur++)
                terrain_table[terrain][cur] = feat;
            if (cur >= MAX_FEAT_IN_TERRAIN) break;

            check++;
        }
        else /* Paranoia */
        {
            plog_fmt("Format error");
        }
    }

    /* Paranoia */
    if (cur < MAX_FEAT_IN_TERRAIN)
    {
        plog_fmt("Too few parameters");
    }

    /* End the varargs stuff */
    va_end(vp);
}


/*
 * Initialize arrays for wilderness terrains
 */
void init_wilderness_terrains(void)
{
    init_terrain_table(TERRAIN_EDGE, feat_permanent, "a",
        feat_permanent, MAX_FEAT_IN_TERRAIN);

    init_terrain_table(TERRAIN_TOWN, feat_town, "a",
        feat_floor, MAX_FEAT_IN_TERRAIN);

    init_terrain_table(TERRAIN_DEEP_WATER, feat_deep_water, "ab",
        feat_deep_water, 12,
        feat_shallow_water, MAX_FEAT_IN_TERRAIN - 12);

    init_terrain_table(TERRAIN_SHALLOW_WATER, feat_shallow_water, "abcde",
        feat_deep_water, 3,
        feat_shallow_water, 12,
        feat_floor, 1,
        feat_dirt, 1,
        feat_grass, MAX_FEAT_IN_TERRAIN - 17);

    init_terrain_table(TERRAIN_SWAMP, feat_swamp, "abcdef",
        feat_dirt, 2,
        feat_grass, 3,
        feat_tree, 1,
        feat_brake, 1,
        feat_shallow_water, 4,
        feat_swamp, MAX_FEAT_IN_TERRAIN - 11);

    init_terrain_table(TERRAIN_DIRT, feat_dirt, "abcdef",
        feat_floor, 3,
        feat_dirt, 10,
        feat_flower, 1,
        feat_brake, 1,
        feat_grass, 1,
        feat_tree, MAX_FEAT_IN_TERRAIN - 16);

    init_terrain_table(TERRAIN_GRASS, feat_grass, "abcdef",
        feat_floor, 2,
        feat_dirt, 2,
        feat_grass, 9,
        feat_flower, 1,
        feat_brake, 2,
        feat_tree, MAX_FEAT_IN_TERRAIN - 16);

    init_terrain_table(TERRAIN_TREES, feat_tree, "abcde",
        feat_floor, 2,
        feat_dirt, 1,
        feat_tree, 11,
        feat_brake, 2,
        feat_grass, MAX_FEAT_IN_TERRAIN - 16);

    init_terrain_table(TERRAIN_DESERT, feat_dirt, "abc",
        feat_floor, 2,
        feat_dirt, 13,
        feat_grass, MAX_FEAT_IN_TERRAIN - 15);

    init_terrain_table(TERRAIN_SHALLOW_LAVA, feat_shallow_lava, "abc",
        feat_shallow_lava, 14,
        feat_deep_lava, 3,
        feat_mountain, MAX_FEAT_IN_TERRAIN - 17);

    init_terrain_table(TERRAIN_DEEP_LAVA, feat_deep_lava, "abcd",
        feat_dirt, 3,
        feat_shallow_lava, 3,
        feat_deep_lava, 10,
        feat_mountain, MAX_FEAT_IN_TERRAIN - 16);

    init_terrain_table(TERRAIN_MOUNTAIN, feat_mountain, "abcdef",
        feat_floor, 1,
        feat_brake, 1,
        feat_grass, 2,
        feat_dirt, 2,
        feat_tree, 2,
        feat_mountain, MAX_FEAT_IN_TERRAIN - 8);
}


bool change_wild_mode(void)
{
    int i;
    bool have_pet = FALSE;

    /* It is in the middle of changing map */
    if (p_ptr->leaving) return FALSE;


    if (lite_town || vanilla_town)
    {
        msg_print("No global map.");
        return FALSE;
    }

    if (p_ptr->wild_mode)
    {
        /* Save the location in the global map */
        if (py != p_ptr->wilderness_y || px != p_ptr->wilderness_x)
        {
            p_ptr->wilderness_x = px;
            p_ptr->wilderness_y = py;
            p_ptr->wilderness_dx = 0;
            p_ptr->wilderness_dy = 0;
        }

        /* Give first move to the player */
        p_ptr->energy_need = 0;

        /* Go back to the ordinary map */
        p_ptr->wild_mode = FALSE;

        /* Leaving */
        p_ptr->leaving = TRUE;

        /* Succeed */
        return TRUE;
    }

    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];

        if (!m_ptr->r_idx) continue;
        if (is_pet(m_ptr) && i != p_ptr->riding) have_pet = TRUE;
        if (MON_CSLEEP(m_ptr)) continue;
        if (m_ptr->cdis > MAX_SIGHT) continue;
        if (!is_hostile(m_ptr)) continue;
        if (r_info[m_ptr->r_idx].level < p_ptr->lev - 10) continue;
        msg_print("You cannot enter global map, since there is some monsters nearby!");
        energy_use = 0;
        return FALSE;
    }

    if (have_pet)
    {
        cptr msg = "Do you leave your pets behind? ";

        if (!get_check_strict(msg, CHECK_OKAY_CANCEL))
        {
            energy_use = 0;
            return FALSE;
        }
    }

    /* HACK */
    energy_use = 1000;

    /* Remember the position */
    p_ptr->oldpx = px;
    p_ptr->oldpy = py;

    /* Cancel hex spelling */
    if (hex_spelling_any()) stop_hex_spell_all();

    /* Cancel any special action */
    set_action(ACTION_NONE);

    /* Go into the global map */
    p_ptr->wild_mode = TRUE;

    /* Leaving */
    p_ptr->leaving = TRUE;

    /* Succeed */
    return TRUE;
}
