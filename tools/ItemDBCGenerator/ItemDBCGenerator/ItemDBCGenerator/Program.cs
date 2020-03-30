using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections;

namespace ItemDBCGenerator
{
    class Program
    {
        private static Boolean debug = false;
        /*
        ItemGenerator Inventory[1] 1000000 - 1584340
        ItemGenerator Inventory[2] 1584341 - 2168681
        ItemGenerator Inventory[3] 2168682 - 2753022
        ItemGenerator Inventory[5] 2753023 - 3337363
        ItemGenerator Inventory[6] 3337364 - 3921704
        ItemGenerator Inventory[7] 3921705 - 4506045
        ItemGenerator Inventory[8] 4506046 - 5090386
        ItemGenerator Inventory[9] 5090387 - 5674727
        ItemGenerator Inventory[10] 5674728 - 6259068
        ItemGenerator Inventory[11] 6259069 - 6843409
        ItemGenerator Inventory[12] 6843410 - 7427750
        ItemGenerator Inventory[14] 7427751 - 8012091
        ItemGenerator Inventory[16] 8012092 - 8596432
        */
        private static ItemRange[] RANGES = {
            new ItemRange(new UInt32[] { 1000000, 1584340 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.HEAD),
            new ItemRange(new UInt32[] { 1584341, 2168681 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.NECK),
            new ItemRange(new UInt32[] { 2168682, 2753022 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.SHOULDER),
            new ItemRange(new UInt32[] { 2753023, 3337363 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.CHEST),
            new ItemRange(new UInt32[] { 3337364, 3921704 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.WAIST),
            new ItemRange(new UInt32[] { 3921705, 4506045 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.LEGS),
            new ItemRange(new UInt32[] { 4506046, 5090386 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.FEET),
            new ItemRange(new UInt32[] { 5090387, 5674727 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.WRISTS),
            new ItemRange(new UInt32[] { 5674728, 6259068 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.HANDS),
            new ItemRange(new UInt32[] { 6259069, 6843409 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.FINGER),
            new ItemRange(new UInt32[] { 6843410, 7427750 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.TRINKET),
            new ItemRange(new UInt32[] { 7427751, 8012091 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.SHIELD),
            new ItemRange(new UInt32[] { 8012092, 8596432 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.BACK),
/*
        ItemGenerator Weapon[0] 8596433 - 9180773
        ItemGenerator Weapon[1] 9180774 - 9765114
        ItemGenerator Weapon[2] 9765115 - 10349455
        ItemGenerator Weapon[3] 10349456 - 10933796
        ItemGenerator Weapon[4] 10933797 - 11518137
        ItemGenerator Weapon[5] 11518138 - 12102478
        ItemGenerator Weapon[6] 12102479 - 12686819
        ItemGenerator Weapon[7] 12686820 - 13271160
        ItemGenerator Weapon[8] 13271161 - 13855501
        ItemGenerator Weapon[10] 13855502 - 14439842
        ItemGenerator Weapon[13] 14439843 - 15024183
        ItemGenerator Weapon[15] 15024184 - 15608524
        ItemGenerator Weapon[18] 15608525 - 16192865
        ItemGenerator Weapon[19] 16192866 - 16777206
*/
            new ItemRange(new UInt32[] { 8596433, 9180773 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.AXE_ONEH),
            new ItemRange(new UInt32[] { 9180774, 9765114 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.AXE_TWOH),
            new ItemRange(new UInt32[] { 9765115, 10349455 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.BOW),
            new ItemRange(new UInt32[] { 10349456, 10933796 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.GUN),
            new ItemRange(new UInt32[] { 10933797, 11518137 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.MACE_1H),
            new ItemRange(new UInt32[] { 11518138, 12102478 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.MACE_2H),
            new ItemRange(new UInt32[] { 12102479, 12686819 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.POLEARM),
            new ItemRange(new UInt32[] { 12686820, 13271160 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.SWORD_1H),
            new ItemRange(new UInt32[] { 13271161, 13855501 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.SWORD_2H),
            new ItemRange(new UInt32[] { 13855502, 14439842 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.STAFF),
            new ItemRange(new UInt32[] { 14439843, 15024183}, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.FIST_WEAPON),
            new ItemRange(new UInt32[] { 15024184, 15608524 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.DAGGER),
            new ItemRange(new UInt32[] { 15608525, 16192865 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.CROSSBOW),
            new ItemRange(new UInt32[] { 16192866, 16777206 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.WAND)
        };

        public static void Print(String message, params Object[] arguments)
        {
            Console.WriteLine(String.Format(message, arguments));
        }

        static void Main(string[] args)
        {
            try
            {
                Print("Reading config...");
                Config config = new Config();
                config.loadFile();

                Print("Creating MySQL connection...");
                MySQL mySQL = new MySQL(config);

                Item dbc = new Item();
                Print("Loaded Item.dbc, records: {0}", dbc.header.RecordCount);

                foreach (ItemRange range in RANGES)
                {
                    Constants.WeaponSubclass weapon = range.WeaponSubClass;
                    String type = "";
                    if (weapon != Constants.WeaponSubclass.NULL)
                        type = weapon.ToString();
                    else
                        type = range.ArmorInventoryType.ToString();
                    UInt32[] r = range.Range;
                    Print("{0} type {1} has range:\t\t{2} - {3}", range.ItemClass.ToString(), type, r[0], r[1]);
                }

                List<Item.DBC_Record> newRecords = new List<Item.DBC_Record>();
                Random rand = new Random();
                foreach (ItemRange range in RANGES)
                {
                    int subClass = 0;
                    if (range.ItemClass == Constants.ItemClass.ARMOR)
                        subClass = (int)range.ArmorInventoryType;
                    else
                        subClass = (int)range.WeaponSubClass;
                    Print("Generating: {0}, {1}, {2}...", range.ItemClass.ToString(), range.WeaponSubClass.ToString(), range.ArmorInventoryType.ToString());

                    String query = "";
                    if(range.ItemClass == Constants.ItemClass.ARMOR)
                    query = String.Format(@"
                        SELECT entry,class,subclass,SoundOverrideSubclass,displayid,inventoryType,Material,sheath FROM item_template WHERE
	                        class = '{0}' AND
                            InventoryType = '{1}'
	                        ORDER BY entry;",
                            (int)range.ItemClass, subClass);
                    else
                        query = String.Format(@"
                        SELECT entry,class,subclass,SoundOverrideSubclass,displayid,inventoryType,Material,sheath FROM item_template WHERE
	                        class = '{0}' AND
                            subclass = '{1}'
	                        ORDER BY entry;",
                            (int)range.ItemClass, subClass);

                    UInt32[] ranges = range.Range;
                    var resultSet = mySQL.Query(query).Rows;
                    int count = resultSet.Count - 1;
                    for (uint i = ranges[0]; i < ranges[1]; ++i)
                    {
                        var row = resultSet[rand.Next(0, count)];
                        Item.DBC_Record record = new Item.DBC_Record
                        {
                            ID = i,
                            itemClass = UInt32.Parse(row[1].ToString()),
                            itemSubClass = UInt32.Parse(row[2].ToString()),
                            sound_override_subclassid = Int32.Parse(row[3].ToString()),
                            itemDisplayInfo = UInt32.Parse(row[4].ToString()),
                            inventorySlotID = UInt32.Parse(row[5].ToString()),
                            materialID = Int32.Parse(row[6].ToString()),
                            sheathID = Int32.Parse(row[7].ToString())
                        };
                        newRecords.Add(record);
                        if (debug)
                            Print("[{0},\t{1},\t{2},\t{3},\t{4},\t{5},\t{6},\t{7},\t{8}]", record.ID, record.itemClass,
                                record.itemSubClass, record.sound_override_subclassid, record.itemDisplayInfo,
                                record.inventorySlotID, record.materialID, record.sheathID);
                    }
                }

                Print("Generated {0} new records.", newRecords.Count);

                Print("Saving new Item.dbc...");
                dbc.SaveDBCFile(newRecords.ToArray());

                Print("Done. Program terminating.");
            }
            catch (Exception e)
            {
                Print("ERROR: {0}", e.Message);
            }
        }
    }
}
