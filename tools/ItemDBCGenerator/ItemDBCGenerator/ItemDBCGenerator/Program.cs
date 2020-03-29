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

        private static ItemRange[] RANGES = {
            new ItemRange(new UInt32[] { 1000000, 1999999 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.HEAD),
            new ItemRange(new UInt32[] { 2000000, 2999999 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.NECK),
            new ItemRange(new UInt32[] { 3000000, 3999999 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.SHOULDER),
            new ItemRange(new UInt32[] { 4000000, 4999999 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.CHEST),
            new ItemRange(new UInt32[] { 5000000, 5999999 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.WAIST),
            new ItemRange(new UInt32[] { 6000000, 6999999 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.LEGS),
            new ItemRange(new UInt32[] { 7000000, 7999999 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.FEET),
            new ItemRange(new UInt32[] { 8000000, 8999999 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.WRISTS),
            new ItemRange(new UInt32[] { 9000000, 9999999 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.HANDS),
            new ItemRange(new UInt32[] { 10000000, 10999999 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.FINGER),
            new ItemRange(new UInt32[] { 11000000, 11999999 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.TRINKET),
            new ItemRange(new UInt32[] { 12000000, 12999999 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.SHIELD),
            new ItemRange(new UInt32[] { 13000000, 13999999 }, Constants.ItemClass.ARMOR, Constants.ArmorInventoryType.BACK),


            new ItemRange(new UInt32[] { 14000000, 14999999 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.AXE_ONEH),
            new ItemRange(new UInt32[] { 15000000, 15999999 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.AXE_TWOH),
            new ItemRange(new UInt32[] { 16000000, 16999999 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.BOW),
            new ItemRange(new UInt32[] { 17000000, 17999999 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.GUN),
            new ItemRange(new UInt32[] { 18000000, 18999999 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.MACE_1H),
            new ItemRange(new UInt32[] { 19000000, 19999999 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.MACE_2H),
            new ItemRange(new UInt32[] { 20000000, 20999999 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.POLEARM),
            new ItemRange(new UInt32[] { 21000000, 21999999 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.SWORD_1H),
            new ItemRange(new UInt32[] { 22000000, 22999999 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.SWORD_2H),
            new ItemRange(new UInt32[] { 23000000, 23999999 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.STAFF),
            new ItemRange(new UInt32[] { 24000000, 24999999}, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.FIST_WEAPON),
            new ItemRange(new UInt32[] { 25000000, 25999999 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.DAGGER),
            new ItemRange(new UInt32[] { 26000000, 26999999 }, Constants.ItemClass.WEAPON, Constants.WeaponSubclass.CROSSBOW)
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
